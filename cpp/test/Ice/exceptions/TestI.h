// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class ThrowerI : public Thrower
{
public:

    ThrowerI(const Ice::ObjectAdapterPtr&);

    virtual void shutdown();
    virtual void throwAasA(Ice::Int);
    virtual void throwAorDasAorD(Ice::Int);
    virtual void throwBasA(Ice::Int, Ice::Int);
    virtual void throwCasA(Ice::Int, Ice::Int, Ice::Int);
    virtual void throwBasB(Ice::Int, Ice::Int);
    virtual void throwCasB(Ice::Int, Ice::Int, Ice::Int);
    virtual void throwCasC(Ice::Int, Ice::Int, Ice::Int);
    virtual void throwUndeclaredA(Ice::Int);
    virtual void throwUndeclaredB(Ice::Int, Ice::Int);
    virtual void throwUndeclaredC(Ice::Int, Ice::Int, Ice::Int);
    virtual void throwLocalException();
    virtual void throwNonIceException();

private:

    Ice::ObjectAdapterPtr _adapter;
};

#endif
