// **********************************************************************
//
// Copyright (c) 2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IcePatch2/ClientUtil.h>
#include <IcePatch2/Util.h>
#include <IcePatch2/FileServerI.h>

#ifdef _WIN32
#   include <direct.h>
#endif

using namespace std;
using namespace Ice;
using namespace IcePatch2;

IcePatch2::Patcher::Patcher(const CommunicatorPtr& communicator, const PatcherFeedbackPtr& feedback) :
    _feedback(feedback),
    _dataDir(normalize(communicator->getProperties()->getProperty("IcePatch2.Directory"))),
    _thorough(communicator->getProperties()->getPropertyAsInt("IcePatch2.Thorough") > 0),
    _dryRun(communicator->getProperties()->getPropertyAsInt("IcePatch2.DryRun") > 0),
    _chunkSize(communicator->getProperties()->getPropertyAsIntWithDefault("IcePatch2.ChunkSize", 100000)),
    _decompress(false)
{
    if(_dataDir.empty())
    {
	throw string("no data directory specified");
    }

    if(_chunkSize < 1)
    {
	const_cast<Int&>(_chunkSize) = 1;
    }

#ifdef _WIN32
    char cwd[_MAX_PATH];
    if(_getcwd(cwd, _MAX_PATH) == NULL)
#else
    char cwd[PATH_MAX];
    if(getcwd(cwd, PATH_MAX) == NULL)
#endif
    {
	throw "cannot get the current directory: " + lastError();
    }
    
    const_cast<string&>(_dataDir) = normalize(string(cwd) + '/' + _dataDir);
    
    PropertiesPtr properties = communicator->getProperties();

    const char* endpointsProperty = "IcePatch2.Endpoints";
    const string endpoints = properties->getProperty(endpointsProperty);
    if(endpoints.empty())
    {
	throw string("property `") + endpointsProperty + "' is not set";
    }
    
    const char* idProperty = "IcePatch2.Identity";
    const Identity id = stringToIdentity(properties->getPropertyWithDefault(idProperty, "IcePatch2/server"));
    
    ObjectPrx serverBase = communicator->stringToProxy(identityToString(id) + ':' + endpoints);
    const_cast<FileServerPrx&>(_serverCompress) = FileServerPrx::checkedCast(serverBase->ice_compress(true));
    if(!_serverCompress)
    {
	throw "proxy `" + identityToString(id) + ':' + endpoints + "' is not a file server.";
    }
    const_cast<FileServerPrx&>(_serverNoCompress) = FileServerPrx::checkedCast(_serverCompress->ice_compress(false));
}

IcePatch2::Patcher::~Patcher()
{
    assert(!_decompress);
}

bool
IcePatch2::Patcher::prepare()
{
    _localFiles.clear();

    bool thorough = _thorough;

    if(!thorough)
    {
	try
	{
	    loadFileInfoSeq(_dataDir, _localFiles);
	}
	catch(const string& ex)
	{
	    thorough = _feedback->noFileSummary(ex);
	    if(!thorough)
	    {
		return false;
	    }
	}
    }
    
    if(thorough)
    {
	getFileInfoSeq(_dataDir, _localFiles, false, false, false);
	saveFileInfoSeq(_dataDir, _localFiles);
    }

    FileTree0 tree0;
    getFileTree0(_localFiles, tree0);

    if(tree0.checksum != _serverCompress->getChecksum())
    {
	if(!_feedback->fileListStart())
	{
	    return false;
	}
	
	ByteSeqSeq checksum0Seq = _serverCompress->getChecksum0Seq();
	if(checksum0Seq.size() != 256)
	{
	    throw string("server returned illegal value");
	}
	
	for(int node0 = 0; node0 < 256; ++node0)
	{
	    if(tree0.nodes[node0].checksum != checksum0Seq[node0])
	    {
		FileInfoSeq files = _serverCompress->getFileInfo1Seq(node0);
		
		sort(files.begin(), files.end(), FileInfoLess());
		files.erase(unique(files.begin(), files.end(), FileInfoEqual()), files.end());
		
		set_difference(tree0.nodes[node0].files.begin(),
			       tree0.nodes[node0].files.end(),
			       files.begin(),
			       files.end(),
			       back_inserter(_removeFiles),
			       FileInfoLess());
		
		set_difference(files.begin(),
			       files.end(),
			       tree0.nodes[node0].files.begin(),
			       tree0.nodes[node0].files.end(),
			       back_inserter(_updateFiles),
			       FileInfoLess());
	    }

	    if(!_feedback->fileListProgress((node0 + 1) * 100 / 256))
	    {
		return false;
	    }
	}

	if(!_feedback->fileListEnd())
	{
	    return false;
	}
    }
    
    sort(_removeFiles.begin(), _removeFiles.end(), FileInfoLess());
    sort(_updateFiles.begin(), _updateFiles.end(), FileInfoLess());

    return true;
}

bool
IcePatch2::Patcher::patch()
{
    if(!_removeFiles.empty())
    {
	if(!removeFiles(_removeFiles))
	{
	    return false;
	}

	if(!_dryRun)
	{
	    saveFileInfoSeq(_dataDir, _localFiles);
	}
    }

    if(!_updateFiles.empty())
    {
	if(!updateFiles(_updateFiles))
	{
	    return false;
	}

	if(!_dryRun)
	{
	    saveFileInfoSeq(_dataDir, _localFiles);
	}
    }

    return true;
}

bool
IcePatch2::Patcher::removeFiles(const FileInfoSeq& files)
{
    if(!_dryRun)
    {
	FileInfoSeq::const_iterator p = files.begin();
	
	while(p != files.end())
	{
	    removeRecursive(_dataDir + '/' + p->path);
	    
	    string dir = p->path + '/';
	    
	    do
	    {
		++p;
	    }
	    while(p != files.end() && p->path.size() > dir.size() &&
		  p->path.compare(0, dir.size(), dir) == 0);
	}

	FileInfoSeq newLocalFiles;
	newLocalFiles.reserve(_localFiles.size());

	set_difference(_localFiles.begin(),
		       _localFiles.end(),
		       files.begin(),
		       files.end(),
		       back_inserter(newLocalFiles),
		       FileInfoLess());
	
	_localFiles.swap(newLocalFiles);

	FileInfoSeq newRemoveFiles;

	set_difference(_removeFiles.begin(),
		       _removeFiles.end(),
		       files.begin(),
		       files.end(),
		       back_inserter(newRemoveFiles),
		       FileInfoLess());
	
	_removeFiles.swap(newRemoveFiles);
    }

    return true;
}
    
bool
IcePatch2::Patcher::updateFiles(const FileInfoSeq& files)
{
    if(!_dryRun)
    {
	string pathLog = _dataDir + ".log";
	_updateLog.open(pathLog.c_str());
	if(!_updateLog)
	{
	    throw "cannot open `" + pathLog + "' for writing: " + lastError();
	}
	
	{
	    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	    _decompress = true;
	}
	
	start();
    }

    try
    {
	if(!updateFilesInternal(files))
	{
	    {
		IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
		_decompress = false;
		notify();
	    }
	    
	    getThreadControl().join();
	    
	    return false;
	}
    }
    catch(...)
    {
	{
	    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	    _decompress = false;
	    notify();
	}
	
	getThreadControl().join();
	
	throw;
    }
    
    if(!_dryRun)
    {
	{
	    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	    if(!_decompressException.empty())
	    {
		throw _decompressException;
	    }
	    _decompress = false;
	    notify();
	}
	
	getThreadControl().join();
	
	_updateLog.close();
	
	FileInfoSeq newLocalFiles;
	newLocalFiles.reserve(_localFiles.size());
	
	set_union(_localFiles.begin(),
		  _localFiles.end(),
		  files.begin(),
		  files.end(),
		  back_inserter(newLocalFiles),
		  FileInfoLess());
	
	_localFiles.swap(newLocalFiles);

	FileInfoSeq newUpdateFiles;

	set_difference(_updateFiles.begin(),
		       _updateFiles.end(),
		       files.begin(),
		       files.end(),
		       back_inserter(newUpdateFiles),
		       FileInfoLess());
	
	_updateFiles.swap(newUpdateFiles);
    }

    return true;
}

bool
IcePatch2::Patcher::updateFilesInternal(const FileInfoSeq& files)
{
    FileInfoSeq::const_iterator p;
    
    Long total = 0;
    Long updated = 0;
    
    for(p = files.begin(); p != files.end(); ++p)
    {
	if(p->size > 0) // Regular, non-empty file?
	{
	    total += p->size;
	}
    }
    
    for(p = files.begin(); p != files.end(); ++p)
    {
	if(p->size < 0) // Directory?
	{
	    if(!_dryRun)
	    {
		createDirectoryRecursive(_dataDir + '/' + p->path);
		
		{
		    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
		    _updateLog << *p << endl;
		}
	    }
	}
	else // Regular file.
	{
	    if(!_feedback->patchStart(p->path, p->size, updated, total))
	    {
		return false;
	    }

	    string pathBZ2 = _dataDir + '/' + p->path + ".bz2";
	    ofstream fileBZ2;
	    
	    if(!_dryRun)
	    {
		string dir = getDirname(pathBZ2);
		if(!dir.empty())
		{
		    createDirectoryRecursive(dir);
		}
		
		try
		{
		    removeRecursive(pathBZ2);
		}
		catch(...)
		{
		}
		
		fileBZ2.open(pathBZ2.c_str(), ios::binary);
		if(!fileBZ2)
		{
		    throw "cannot open `" + pathBZ2 + "' for writing: " + lastError();
		}
	    }
	    
	    Int pos = 0;
	    string progress;
	    
	    while(pos < p->size)
	    {
		ByteSeq bytes;
		
		try
		{
		    bytes = _serverNoCompress->getFileCompressed(p->path, pos, _chunkSize);
		}
		catch(const FileAccessException& ex)
		{
		    throw "server error for `" + p->path + "':" + ex.reason;
		}
		
		if(bytes.empty())
		{
		    throw "size mismatch for `" + p->path + "'";
		}
		
		if(!_dryRun)
		{
		    fileBZ2.write(reinterpret_cast<char*>(&bytes[0]), bytes.size());
		    
		    if(!fileBZ2)
		    {
			throw ": cannot write `" + pathBZ2 + "': " + lastError();
		    }
		}
		
		pos += bytes.size();
		updated += bytes.size();
		
		if(!_feedback->patchProgress(pos, p->size, updated, total))
		{
		    return false;
		}
	    }
	    
	    if(!_dryRun)
	    {
		fileBZ2.close();
		
		{
		    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
		    if(!_decompressException.empty())
		    {
			throw _decompressException;
		    }
		    _decompressList.push_back(*p);
		    notify();
		}
	    }
	
	    if(!_feedback->patchEnd())
	    {
		return false;
	    }
	}
    }

    return true;
}

void
IcePatch2::Patcher::run()
{
    FileInfo info;
    
    while(true)
    {
	{
	    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	    
	    if(!info.path.empty())
	    {
		_updateLog << info << endl;
	    }
	    
	    while(_decompress && _decompressList.empty())
	    {
		wait();
	    }
	    
	    if(!_decompressList.empty())
	    {
		info = _decompressList.front();
		_decompressList.pop_front();
	    }
	    else
	    {
		return;
	    }
	}
	
	try
	{
	    decompressFile(_dataDir + '/' + info.path);
	    remove(_dataDir + '/' + info.path + ".bz2");
	}
	catch(const string& ex)
	{
	    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	    _decompress = false;
	    _decompressException = ex;
	    return;
	}
    }
}
