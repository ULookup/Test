#pragma once
#include<functional>
namespace Icepop
{
    template<class T>
    class shared_ptr
    {
     public:
	 shared_ptr(T* ptr)
	     :_ptr(ptr),
	     _pcount(new int(1))
	{}
	 template<class D>
	 shared_ptr(T* ptr,D del)
	     :_ptr(ptr),
	     _pcount(new int(1)),
	     _del(del)
	{}
	 shared_ptr(const shared_ptr<T>& sp)
	     :_ptr(sp._ptr),
	      _pcount(sp._pcount),
	      _del(sp._del)
	{
	    ++(*_pcount);
	}
	 shared_ptr<T>& operator=(const shared_ptr<T>& sp)
	 {
	     if(_ptr != sp._ptr)
	     {
		 release();
                 _ptr = sp._ptr;
		 _pcount = sp._pcount;
		 (_pcount++);
		 _del = sp._del;
	     }
	     return *this;
	 }
	 void release()
	 {
	     if(--(*_pcount)==0)
	     {
		 _del(_ptr);
		 delete _pcount;
		 _ptr = nullptr;
		 _pcount = nullptr;
	     }
	 }
	 ~shared_ptr()
	 {
	     release();
	 }
	 T* get() const
	 {
	     return _ptr;
	 }
	 int use_count() const
	 {
	     return *_pcount;
	 }
	 T& operator*()
	 {
	     return *_ptr;
	 }
	 T* operator->()
	 {
	     return _ptr;
	 }
     private:
	 T* _ptr;
	 int* _pcount;
	 std::function<void(T*)> _del = [](T* ptr){delete ptr; };
    };
}
