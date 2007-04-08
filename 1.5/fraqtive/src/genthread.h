/**************************************************************************
* This file is part of the Fraqtive program.
* Copyright (C) 2005 Michal Mecinski.
* This program is licensed under the GNU General Public License.
**************************************************************************/

#ifndef GENTHREAD_H
#define GENTHREAD_H

#include <qptrlist.h>
#include <qthread.h>
#include <qwaitcondition.h>
#include <qmutex.h>

class Generator;

/*!
* \brief The thread used for generating data asynchronously.
*/
class GeneratorThread : public QThread
{
public:
	/*!
	* \brief Constructor.
	*/
	GeneratorThread();

	/*!
	* \brief Destructor.
	*/
	~GeneratorThread();

public:
	/*!
	* \brief Wake the thread to check its generator queue.
	*/
	void wakeUp();

	/*!
	* \brief Notify and wait for the thread to end.
	*/
	void abort();

	/*!
	* \brief Add the generator to the queue and wake the thread.
	*
	* \param generator    The generator to process.
	* \param highPriority If \c true, the generator is added to the front of
	*                     the queue. If \c false, the generator is added to the back.
	*/
	void addToQueue(Generator* generator, bool highPriority);

protected:
	void run();

private:
	typedef QPtrList<Generator> GeneratorQueue;

	bool _abort;

	// a condition to put the thread to sleep
	QWaitCondition _condition;

	// a protection mutex for the queue
	QMutex _queueMutex;

	// the generator queue to process
	GeneratorQueue _queue;
};

extern GeneratorThread generatorThread;

#endif
