// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

class CommunicatorI extends LocalObjectImpl implements Communicator
{
    public synchronized void
    destroy()
    {
        if(!_destroyed) // Don't destroy twice.
        {
	    _destroyed = true;

            _instance.objectAdapterFactory().shutdown();
            _instance.destroy();

	    _serverThreadPool = null;
        }
    }

    public void
    shutdown()
    {
	//
	// No mutex locking here!
	//
	if(_serverThreadPool != null)
	{
	    _serverThreadPool.initiateShutdown();
	}
    }

    public void
    waitForShutdown()
    {
        //
        // No mutex locking here, otherwise the communicator is blocked
        // while waiting for shutdown.
        //
	if(_serverThreadPool != null)
	{
	    _serverThreadPool.waitUntilFinished();
	}
    }

    public synchronized Ice.ObjectPrx
    stringToProxy(String s)
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        return _instance.proxyFactory().stringToProxy(s);
    }

    public synchronized String
    proxyToString(Ice.ObjectPrx proxy)
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        return _instance.proxyFactory().proxyToString(proxy);
    }

    public synchronized ObjectAdapter
    createObjectAdapter(String name)
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }

	ObjectAdapter adapter;

	if(name.length() == 0)
	{
	    adapter = _instance.objectAdapterFactory().createObjectAdapter("", "", "");
	}
	else
	{
	    String id = _instance.properties().getProperty(name + ".AdapterId");

	    String endpts = _instance.properties().getProperty(name + ".Endpoints");

	    adapter = _instance.objectAdapterFactory().createObjectAdapter(name, endpts, id);

	    String router = _instance.properties().getProperty("Ice.Adapter." + name + ".Router");
	    if(router.length() > 0)
	    {
		adapter.addRouter(RouterPrxHelper.uncheckedCast(_instance.proxyFactory().stringToProxy(router)));
	    }

	    String locator = _instance.properties().getProperty("Ice.Adapter." + name + ".Locator");
	    if(locator.length() > 0)
	    {
		adapter.setLocator(LocatorPrxHelper.uncheckedCast(_instance.proxyFactory().stringToProxy(locator)));
	    }
	    else
	    {
		adapter.setLocator(_instance.referenceFactory().getDefaultLocator());
	    }
	}

	if(_serverThreadPool == null) // Lazy initialization of _serverThreadPool.
	{
	    _serverThreadPool = _instance.serverThreadPool();
	}

        return adapter;
    }

    public synchronized ObjectAdapter
    createObjectAdapterWithEndpoints(String name, String endpoints)
    {
	getProperties().setProperty(name + ".Endpoints", endpoints);
	return createObjectAdapter(name);
    }

    public synchronized void
    addObjectFactory(ObjectFactory factory, String id)
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        _instance.servantFactoryManager().add(factory, id);
    }

    public synchronized void
    removeObjectFactory(String id)
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        _instance.servantFactoryManager().remove(id);
    }

    public synchronized ObjectFactory
    findObjectFactory(String id)
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        return _instance.servantFactoryManager().find(id);
    }

    public synchronized void
    addUserExceptionFactory(UserExceptionFactory factory, String id)
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        _instance.userExceptionFactoryManager().add(factory, id);
    }

    public synchronized void
    removeUserExceptionFactory(String id)
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        _instance.userExceptionFactoryManager().remove(id);
    }

    public synchronized UserExceptionFactory
    findUserExceptionFactory(String id)
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        return _instance.userExceptionFactoryManager().find(id);
    }

    public synchronized Properties
    getProperties()
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        return _instance.properties();
    }

    public synchronized Logger
    getLogger()
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        return _instance.logger();
    }

    public synchronized void
    setLogger(Logger logger)
    {
        if(_destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        _instance.logger(logger);
    }

    public void
    setDefaultRouter(RouterPrx router)
    {
        _instance.referenceFactory().setDefaultRouter(router);
    }

    public void
    setDefaultLocator(LocatorPrx locator)
    {
        _instance.referenceFactory().setDefaultLocator(locator);
    }

    public PluginManager
    getPluginManager()
    {
        return null;
    }

    CommunicatorI(StringSeqHolder args, Properties properties)
    {
	_destroyed = false;
        _instance = new IceInternal.Instance(this, args, properties);
    }

    protected void
    finalize()
        throws Throwable
    {
        if(!_destroyed)
        {
            _instance.logger().warning("Ice::Communicator::destroy() has not been called");
        }

        super.finalize();
    }

    //
    // Certain initialization tasks need to be completed after the
    // constructor.
    //
    void
    finishSetup(StringSeqHolder args)
    {
        _instance.finishSetup(args);
    }

    //
    // For use by Util.getInstance()
    //
    IceInternal.Instance
    getInstance()
    {
        return _instance;
    }

    private boolean _destroyed;
    private IceInternal.Instance _instance;

    //
    // We need _serverThreadPool directly in CommunicatorI. That's
    // because the shutdown() operation is signal-safe, and thus must
    // not access any mutex locks or _instance. It may only access
    // _serverThreadPool->initiateShutdown(), which is signal-safe as
    // well.
    //
    private IceInternal.ThreadPool _serverThreadPool;
}
