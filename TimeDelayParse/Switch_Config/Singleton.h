#ifndef		_SINGLETON_H_
#define		_SINGLETON_H_

/**************************************************************************
 * \file    singleton.h
 * \brief   
 *
 * Copyright (c) .
 * RCS: $Id: singleton.h,v 1.9 2008/08/06 10:39:17 leiq Exp $
 *
 * History
 *  2006/10/11 leiq     created
**************************************************************************/

template< typename T >
class Singleton
{
    public:
        static T *instance()
        {
            static T instance_;
            return &instance_;
        }	
    private:
        Singleton( const Singleton& );
        Singleton& operator = ( const Singleton& );
        Singleton();
};

//            static ThreadMutex  m_mutex;
//            ThreadLocker locker(m_mutex);
//template <typename T> Singleton<T>::Tmp_Obj Singleton<T>::t;
/*
				struct Tmp_Obj
				{
					public:
					Tmp_Obj(){Singleton<T>::instance();};
					inline void donothing(){};
				};
				static Tmp_Obj t;	
*/
	


#endif

