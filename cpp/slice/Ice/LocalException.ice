// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_LOCAL_EXCEPTION_ICE
#define ICE_LOCAL_EXCEPTION_ICE

module Ice
{

/**
 *
 * This exception is raised if an operation call on a server raises a
 * local exception. Since the exception is local, it is not
 * transmitted by the Ice protocol. Instead, the client only receives
 * an [UknownLocalException] for all local exceptions being raised by
 * the server.
 *
 **/
local exception UnknownLocalException
{
};

/**
 *
 * This exception is raised if an operation call on a server raises a
 * user exception which is not declared in the exception's
 * <literal>throws</literal> clause. Such undeclared exceptions are
 * not transmitted from the server to the client by the Ice protocol,
 * but instead the client just gets an [UnknownUserException]. This is
 * necessary in order to not violate the contract established by an
 * operation's signature: Only local exceptions and user exceptions
 * declared in the <literal>throws</literal> clause can be raised.
 *
 **/
local exception UnknownUserException
{
};

/**
 *
 * This exception is raised if an operation call on a server raises an
 * unknown exception. For example, for C++, this exception is raised
 * if the server throws a C++ exception that is not directly or
 * indirectly derived from <literal>Ice::LocalException</literal> or
 * <literal>Ice::UserException</literal>.
 *
 **/
local exception UnknownException
{
};

/**
 *
 * This exception is raised if the Ice library version doesn't match
 * the Ice header files version.
 *
 **/
local exception VersionMismatchException
{
};

/**
 *
 * This exception is raised if the [Communicator] has been destroyed.
 *
 * @see Communicator::destroy
 *
 **/
local exception CommunicatorDestroyedException
{
};

/**
 *
 * This exception is raised if an attempt is made to use a deactivated
 * [ObjectAdapter].
 *
 * @see ObjectAdapter::deactivate
 * @see Communicator::shutdown
 *
 **/
local exception ObjectAdapterDeactivatedException
{
};

/**
 *
 * This exception is raised if not suitable endpoint is available in
 * an object reference.
 *
 **/
local exception NoEndpointException
{
};

/**
 *
 * This exception is raised if there was an error while parsing an
 * endpoint.
 *
 **/
local exception EndpointParseException
{
};

/**
 *
 * This exception is raised if there was an error while parsing an
 * object reference.
 *
 **/
local exception ReferenceParseException
{
};

/**
 *
 * This exception is raised if an operation call using a Proxy
 * resulted in a location forward to an object reference that doesn't
 * match the Proxy's identity.
 *
 **/
local exception ReferenceIdentityException
{
};

/**
 *
 * This exception is raised if an object does not exist on the server.
 *
 * @see ObjectAdapter::add
 * @see ObjectAdapter::addServantLocator
 *
 **/
local exception ObjectNotExistException
{
};

/**
 *
 * This exception is raised if an operation for a given object does
 * not exist on the server. Typically this is caused by either the
 * client or the server using an outdated Slice specification.
 *
 **/
local exception OperationNotExistException
{
};

/**
 *
 * This exception is raised if a system error occurred in the server
 * or client process. There are many possible causes for such a system
 * exception. For details on the cause, [SystemException::error]
 * should be inspected.
 *
 **/
local exception SystemException
{
    /**
     *
     * The error number describing the system exception. For C++ and
     * Unix, this is equivalent to <literal>errno</literal>. For C++
     * and Windows, this is the value returned by
     * <literal>GetLastError()</literal> or
     * <literal>WSAGetLastError()</literal>.
     *
     **/
    int error; // Don't use errno, as errno is usually a macro.
};

/**
 *
 * This exception is a specialization of [SystemException] for socket
 * errors.
 *
 **/
local exception SocketException extends SystemException
{
};

/**
 *
 * This exception is a specialization of [SocketException] for
 * connection failures.
 *
 **/
local exception ConnectFailedException extends SocketException
{
};

/**
 *
 * This exception is a specialization of [SocketException], indicating
 * a lost connection.
 *
 **/
local exception ConnectionLostException extends SocketException
{
};

/**
 *
 * This exception indicates a DNS problem. For details on the cause,
 * [DNSException::error] should be inspected.
 *
 **/
local exception DNSException
{
    /**
     *
     * The error number describing the DNS problem. For C++ and Unix,
     * this is equivalent to <literal>h_errno</literal>. For C++ and
     * Windows, this is the value returned by
     * <literal>WSAGetLastError()</literal>.
     *
     **/
    int error; // Don't use h_errno, as h_errno is usually a macro.
};

/**
 *
 * This exception indicates a timeout condition.
 *
 **/
local exception TimeoutException
{
};

/**
 *
 * This exception is a specialization of [TimeoutException] for
 * connection timeout conditions.
 *
 **/
local exception ConnectTimeoutException extends TimeoutException
{
};

/**
 *
 * A generic exception base for all kinds of protocol error
 * conditions.
 *
 **/
local exception ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException], which is
 * raised if no suitable servant factory was found during unmarshaling
 * of a Servant type with operations.
 *
 * @see ServantFactory
 * @see Communicator::addServantFactory
 * @see Communicator::removeServantFactory
 * @see Communicator::findServantFactory
 *
 **/
local exception NoServantFactoryException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException], which is
 * raised if a servant type is encountered that is either unknown or
 * doesn't match the operation signature.
 *
 **/
local exception ServantUnmarshalException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException], which is
 * raised if a user exception is encountered that is either unknown or
 * doesn't match the operation signature.
 *
 **/
local exception UserExceptionUnmarshalException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException], which is
 * raised if an out-of-bounds condition occurs during unmarshaling.
 *
 **/
local exception UnmarshalOutOfBoundsException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating a malformed string encoding.
 *
 **/
local exception StringEncodingException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException], which is
 * raised if the system-specific memory limit is exceeded during
 * marshaling or unmarshaling.
 *
 **/
local exception MemoryLimitException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating a malformed data encapsulation.
 *
 **/
local exception EncapsulationException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating that an unsupported protocol version has been
 * encountered.
 *
 **/
local exception UnsupportedProtocolException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating that an unsupported data encoding version has been
 * encountered.
 *
 **/
local exception UnsupportedEncodingException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating that an invalid protocol message has been received.
 *
 **/
local exception InvalidMessageException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating that an unknown protocol message has been received.
 *
 **/
local exception UnknownMessageException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating that a response for an unknown request id has been
 * received.
 *
 **/
local exception UnknownRequestIdException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating that an unknown reply status been received.
 *
 **/
local exception UnknownReplyStatusException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating that an exception has been closed by the peer.
 *
 **/
local exception CloseConnectionException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating that a batch request has been aborted.
 *
 **/
local exception AbortBatchRequestException extends ProtocolException
{
};

};

#endif
