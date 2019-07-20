#include <iostream>
#include <list>
#include <typeinfo>
#include <cstdlib>
#include "gc_details.h"
#include "gc_iterator.h"

template <class T, int size = 0>
class Pointer{
private:

    static std::list<PtrDetails<T> > refContainer;

	T *addr;

	bool isArray;

	unsigned arraySize;

	static bool first;

	typename std::list<PtrDetails<T> >::iterator findPtrInfo(T *ptr);

public:
    typedef Iter<T> GCiterator;

    Pointer(){
        Pointer(NULL);
    }
    Pointer(T*);

    Pointer(const Pointer &);

    ~Pointer();

    static bool collect();

    T *operator=(T *t);

    Pointer &operator=(Pointer &rv);

    T &operator*(){
        return *addr;
    }

    T *operator->() { return addr; }

    T &operator[](int i){ return addr[i];}

    operator T *() { return addr; }

    Iter<T> begin(){
        int _size;
        if (isArray)
            _size = arraySize;
        else
            _size = 1;
        return Iter<T>(addr, addr, addr + _size);
    }

    Iter<T> end(){
        int _size;
        if (isArray)
            _size = arraySize;
        else
            _size = 1;
        return Iter<T>(addr + _size, addr, addr + _size);
    }

    static int refContainerSize() { return refContainer.size(); }

    static void showlist();

    static void shutdown();
};

// STATIC INITIALIZATION
// Creates storage for the static variables
template <class T, int size>
std::list<PtrDetails<T> > Pointer<T, size>::refContainer;
template <class T, int size>
bool Pointer<T, size>::first = true;


template<class T,int size>
Pointer<T,size>::Pointer(T *t){
    if (this->first)
        atexit(shutdown);
    this->first = false;

    this->addr = t;
    if (size != 0) this->isArray = true;
    else this->isArray = false;

    this->arraySize = size;
    delete t;
}

// Copy constructor.
template< class T, int size>
Pointer<T,size>::Pointer(const Pointer &ob){
    std::cout << "Copy Const 1 " << "\n";
    typename std::list<PtrDetails<T> >::iterator p;
    p = findPtrInfo(ob.addr);
    p->refcount++;
    addr = ob.addr;
    arraySize = ob.arraySize;
    isArray = ob.isArray;
    delete ob.addr;
}

// Destructor for Pointer.
template <class T, int size>
Pointer<T, size>::~Pointer(){
    typename std::list<PtrDetails<T> >::iterator p;
    std::cout << "~Pointer 1 " << "\n";
    p = findPtrInfo(this->addr);
	if (p->refcount) {
    	p->refcount--;
    }
    collect();
}

// Collect garbage
template <class T, int size>
bool Pointer<T, size>::collect(){
    bool memfreed = false;
    typename std::list<PtrDetails<T> >::iterator p;
    do{
        for (p = refContainer.begin(); p != refContainer.end(); p++){
            std::cout << "Running through refContainer" << "\n";
			if (p->refcount > 0)
				continue;
			memfreed = true;
			refContainer.remove(*p);
			std::cout << p->memPtr << "\n";
            if (p->memPtr) {
				if (p->isArray) {
					delete[] p->memPtr;
				} else {
					delete p->memPtr;
				}
			}
            break;
        }
    } while (p != refContainer.end());
    return memfreed;
}

// Overload assignment of pointer to Pointer.
template <class T, int size>
T *Pointer<T, size>::operator=(T *t){
	typename std::list<PtrDetails<T> >::iterator p;

	std::cout << "overloading 1 " << "\n";

	p = findPtrInfo(this->addr);
    if (p->refcount) {
        p->refcount--;
    }

    PtrDetails<T> pDetails (t, 0);
	pDetails.memPtr = t;
    pDetails.isArray = false;

	refContainer.push_back(pDetails);

    this->addr = t;
    this->isArray = false;
    this->arraySize = 0;

    return addr;
}

// Overload assignment of Pointer to Pointer.
template <class T, int size>
Pointer<T, size> &Pointer<T, size>::operator=(Pointer &rv){
    std::cout << "overloading 2 " << "\n";
	typename std::list<PtrDetails<T> >::iterator p;

	p = findPtrInfo(this->addr);
    p->refcount--;

    p = findPtrInfo(rv.addr);
    p->refcount++;
    this->addr = rv.addr;

    delete rv;
}

// A utility function that displays refContainer.
template <class T, int size>
void Pointer<T, size>::showlist(){
    typename std::list<PtrDetails<T> >::iterator p;
    std::cout << "refContainer<" << typeid(T).name() << ", " << size << ">:\n";
    std::cout << "memPtr refcount value\n ";
    if (refContainer.begin() == refContainer.end())
    {
        std::cout << " Container is empty!\n\n ";
    }
    for (p = refContainer.begin(); p != refContainer.end(); p++)
    {
        std::cout << "[" << (void *)p->memPtr << "]"
             << " " << p->refcount << " ";
        if (p->memPtr)
            std::cout << " " << *p->memPtr;
        else
            std::cout << "---";
        std::cout << std::endl;
    }
    std::cout << std::endl;
}
// Find a pointer in refContainer.
template <class T, int size>
typename std::list<PtrDetails<T> >::iterator
Pointer<T, size>::findPtrInfo(T *ptr){
    typename std::list<PtrDetails<T> >::iterator p;
    // Find ptr in refContainer.
    for (p = refContainer.begin(); p != refContainer.end(); p++)
        if (p->memPtr == ptr)
            return p;
    return p;
}
// Clear refContainer when program exits.
template <class T, int size>
void Pointer<T, size>::shutdown(){
    if (refContainerSize() == 0)
        return; // list is empty
    typename std::list<PtrDetails<T> >::iterator p;
    for (p = refContainer.begin(); p != refContainer.end(); p++)
    {
        // Set all reference counts to zero
        p->refcount = 0;
    }
    collect();
}
