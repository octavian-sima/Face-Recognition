/* 
 * File:   CppInterfaces.h
 * Author: Octavian Sima 
 * Thanks: Jose Lamas Rios: http://www.codeproject.com/KB/cpp/CppInterfaces.aspx
 * 
 * Defines a simple way to declare interfaces in C++ in OOP style (like Java, C#)
 *
 * Example:
 * DeclareInterface(IPerson)
 *      virtual string getName() const = 0;
 *      virtual void setName(string name) = 0;
 * EndInterface
 */

#ifndef _CPPINTERFACES_H
#define	_CPPINTERFACES_H

//an interface is a pure virtual  class
#define Interface class

//interface methods must be public
#define implements public

//every class implementing an interface must declare a destructor
#define DeclareInterface(name) Interface name { \
          public: \
          virtual ~name() {}

//base interface
#define DeclareBasedInterface(name, base) class name : public base { \
          public: \
          virtual ~name() {}

//end of interface declaration
#define EndInterface };

#endif	/* _CPPINTERFACES_H */

