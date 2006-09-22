/**************************************************************************
* This file is part of the Fraqtive program.
* Copyright (C) 2005 Michal Mecinski.
* This program is licensed under the GNU General Public License.
**************************************************************************/

#include "genthread.h"
#include "generator.h"

GeneratorThread generatorThread;

GeneratorThread::GeneratorThread()
{
}

GeneratorThread::~GeneratorThread()
{
}

void GeneratorThread::run()
{
	_queueMutex.lock();

	for (;;) {
		while (!_queue.isEmpty()) {
			Generator* generator = _queue.first();
			_queue.removeFirst();
			_queueMutex.unlock();
			generator->generate();
			_queueMutex.lock();
		}

		if (_abort)
			break;

		_condition.wait(&_queueMutex);
	}
	
	_queueMutex.unlock();
}

void GeneratorThread::wakeUp()
{
	_condition.wakeOne();
}

void GeneratorThread::abort()
{
	_queueMutex.lock();
	_queue.clear();
	_queueMutex.unlock();
	_abort = true;
	_condition.wakeOne();
	wait();
}

void GeneratorThread::addToQueue(Generator* generator, bool highPriority)
{
	_queueMutex.lock();
	if (highPriority)
		_queue.prepend(generator);
	else
		_queue.append(generator);
	_queueMutex.unlock();
	_condition.wakeOne();
}
