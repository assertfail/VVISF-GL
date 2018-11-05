#ifndef AUDIOCONTROLLER_H
#define AUDIOCONTROLLER_H

#include <QObject>
#include <QScopedPointer>
#include <QAudioInput>
#include <QAudioDeviceInfo>
#include <QLinkedList>

#include <mutex>

#include "VVGL.hpp"
#include "ISFAudioBufferList.h"




using namespace VVGL;




class AudioController : public QObject
{
	Q_OBJECT
public:
	explicit AudioController(QObject *parent = nullptr);
	
	QList<QAudioDeviceInfo> getListOfDevices();
	QAudioDeviceInfo * currentDeviceInfo();
	void updateAudioResults();
	GLBufferRef getAudioImageBuffer(const int & inWidth = 0);
	GLBufferRef getAudioFFTBuffer(const int & inWidth = 0);

signals:
	Q_SIGNAL void listOfAudioDevicesUpdated();

public slots:

private:
	std::recursive_mutex			_lock;
	
	GLCPUToTexCopierRef				_audioUploader;
	
	QScopedPointer<QAudioDeviceInfo>	_device;
	QAudioFormat					_format;
	QScopedPointer<QAudioInput>		_input;
	
	QLinkedList<ISFAudioBufferList>		_bufferList;
	ISFAudioBufferList				_lastABL;	//	the last ABL we processed
	GLBufferRef						_audioBuffer = nullptr;
	GLBufferRef						_fftBuffer = nullptr;
};




AudioController * GetAudioController();




#endif // AUDIOCONTROLLER_H