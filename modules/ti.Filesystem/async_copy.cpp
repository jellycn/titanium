/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "async_copy.h"
#include "filesystem_binding.h"

namespace ti
{
	AsyncCopy::AsyncCopy(FilesystemBinding* parent,Host *host,std::vector<std::string> files, std::string destination, SharedBoundMethod callback) :
		parent(parent), host(host), files(files), destination(destination), callback(callback), stopped(false)
	{
		this->Set("running",Value::NewBool(true));
		this->thread = new Poco::Thread();
		this->thread->start(&AsyncCopy::Run,this);
	}
	AsyncCopy::~AsyncCopy()
	{
		KR_DUMP_LOCATION
		if (this->thread!=NULL)
		{
			std::cout << "~AsyncCopy:: before delete thread" << std::endl;
			this->thread->tryJoin(10);
			delete this->thread;
			std::cout << "~AsyncCopy:: after delete thread" << std::endl;
			this->thread = NULL;
		}
		std::cout << "after ~AsyncCopy" << std::endl;
	}
	void AsyncCopy::Run(void* data)
	{
#ifdef OS_OSX
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
#endif

		AsyncCopy* ac = static_cast<AsyncCopy*>(data);
#ifdef DEBUG
		std::cout << "async copy started with " << ac->files.size() << " files" << std::endl;
#endif
		std::vector<std::string>::iterator iter = ac->files.begin();
		int c = 0;
		while(!ac->stopped && iter!=ac->files.end())
		{
			std::string file = (*iter++);
			c++;
#ifdef DEBUG
			std::cout << "copying async file: " << file << " (" << c << "/" << ac->files.size() << ")" << std::endl;
#endif
			try
			{
				Poco::File from(file);
				from.copyTo(ac->destination);
#ifdef DEBUG
			std::cout << "copied async file: " << file << " (" << c << "/" << ac->files.size() << ")" << std::endl;
#endif
				SharedValue value = Value::NewString(file);
				ValueList *args = new ValueList;
				args->push_back(value);
				args->push_back(Value::NewInt(c));
				args->push_back(Value::NewInt(ac->files.size()));
				SharedPtr<ValueList> a(args);
				ac->host->InvokeMethodOnMainThread(ac->callback,a);
#ifdef DEBUG
			std::cout << "after callback for file #" << c << std::endl;
#endif
			}
			catch (Poco::Exception &ex)
			{
				std::cerr << "Error running async file copy: " << ex.displayText() << std::endl;
			}
			catch (std::exception &ex)
			{
				std::cerr << "Error running async file copy: " << ex.what() << std::endl;
			}
			catch (...)
			{
				std::cerr << "Unknown error running async file copy" << std::endl;
			}
		}
		ac->Set("running",Value::NewBool(false));
		ac->stopped = true;
#ifdef DEBUG
		std::cout << "async copy finished by copying " << c << " files" << std::endl;
#endif
		std::cerr << "+++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
#ifdef OS_OSX
		[pool release];
#endif
	}
	void AsyncCopy::ToString(const ValueList& args, SharedValue result)
	{
		result->SetString("[Async Copy]");
	}
	void AsyncCopy::Cancel(const ValueList& args, SharedValue result)
	{
		KR_DUMP_LOCATION
		if (thread!=NULL && thread->isRunning())
		{
			this->stopped = true;
			thread->tryJoin(100); // just wait a brief amount to not lock up JS thread
			result->SetBool(true);
			this->Set("running",Value::NewBool(false));
		}
		else
		{
			result->SetBool(false);
		}
	}
}