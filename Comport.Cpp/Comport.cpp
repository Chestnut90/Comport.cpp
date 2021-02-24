#include "pch.h"
#include "Comport.h"

Comport::Comport()
{
	
}

Comport::Comport(CString comportName, CString baudRates, CString parity, CString dataBits, CString stopBits)
{
	this->ComportName = comportName;
	this->BaudRates = baudRates;
	this->Parity = parity;
	this->DataBits = dataBits;
	this->StopBits = stopBits;

	// to move
	this->length = 0;
	this->isFlowChecked = true;
	this->isOpened = false;
	this->_event = new CEvent(false, true);

}

Comport::~Comport()
{
	this->Close();
	delete this->_event;
}

bool Comport::IsOpened()
{
	return this->isOpened;
}

bool Comport::Open()
{
	// Open
	// create file
	this->comportDevice = CreateFile(this->ComportName,
		GENERIC_READ | GENERIC_WRITE,
		0, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);

	if (this->comportDevice == INVALID_HANDLE_VALUE) 
	{
		return false;
	}
	this->isOpened = true;	// opened;

	this->ResetSerial();	//?

	// read overlapped setting
	this->read.Offset = 0;
	this->read.OffsetHigh = 0;
	this->read.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// write overlapped setting
	this->write.Offset = 0;
	this->write.OffsetHigh = 0;
	this->write.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// not created
	if (this->read.hEvent == NULL || this->write.hEvent == NULL)
	{
		CloseHandle(this->read.hEvent);
		CloseHandle(this->write.hEvent);
		return false;
	}

	// TODO threading
	EscapeCommFunction(this->comportDevice, SETDTR);	//?
	return true;
}

void Comport::Close()
{
	if (!this->isOpened) 
	{
		return;
	}

	this->isOpened = false;
	SetCommMask(this->comportDevice, 0);	// ?
	EscapeCommFunction(this->comportDevice, CLRDTR);	//?
	PurgeComm(this->comportDevice,
		PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);	//?
	Sleep(500);
}

int Comport::Receive(char* inBuffer, int len)
{
	CSingleLock lockObj((CSyncObject*)this->_event, FALSE);
	// argument value is not valid
	if (len == 0)
		return -1;
	else if (len > MAX_BUFFER)
		return -1;

	if (this->length == 0) {
		this->inBuffer[0] = '\0';
		return 0;
	}
	else if (this->length <= len) {
		lockObj.Lock();
		memcpy(inBuffer, this->inBuffer, this->length);
		memset(this->inBuffer, 0, MAX_BUFFER * 2);
		int tmp = this->length;
		this->length = 0;
		lockObj.Unlock();
		return tmp;
	}
	else {
		lockObj.Lock();
		memcpy(inBuffer, this->inBuffer, len);
		memmove(this->inBuffer, this->inBuffer + len, MAX_BUFFER * 2 - len);
		this->length -= len;
		lockObj.Unlock();
		return len;
	}
}

bool Comport::Send(const char* outBuffer, int len)
{
	bool res = true;
	unsigned long errorFlags;	// DWORD
	COMSTAT comstat;

	unsigned long numberOfBytesToWrite;
	//unsigned long 

	ClearCommError(this->comportDevice, &errorFlags, &comstat);
	if (!WriteFile(this->comportDevice, outBuffer, len, &numberOfBytesToWrite, &this->write)) {
		if (GetLastError() == ERROR_IO_PENDING) {
			if (WaitForSingleObject(this->write.hEvent, 1000) != WAIT_OBJECT_0)
				res = FALSE;
			else
				GetOverlappedResult(this->comportDevice, &this->write, &numberOfBytesToWrite, FALSE);
		}
		else
		{
			/* I/O error */
			res = FALSE; /* ignore error */
		}
	}

	ClearCommError(this->comportDevice, &errorFlags, &comstat);

	return res;
}

void Comport::HandleClose()
{
	CloseHandle(this->comportDevice);
	CloseHandle(this->read.hEvent);
	CloseHandle(this->write.hEvent);
}

void Comport::ResetSerial()
{
	DCB		dcb;
	DWORD	DErr;
	COMMTIMEOUTS	thisTimeOuts;

	if (!this->isOpened)
		return;

	ClearCommError(this->comportDevice, &DErr, NULL);
	SetupComm(this->comportDevice, IN_BUFFER_SIZE, OUT_BUFFER_SIZE);
	PurgeComm(this->comportDevice, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

	// set up for overlapped I/O
	thisTimeOuts.ReadIntervalTimeout = MAXDWORD;
	thisTimeOuts.ReadTotalTimeoutMultiplier = 0;
	thisTimeOuts.ReadTotalTimeoutConstant = 0;

	// CBR_9600 is approximately 1byte/ms. For our purposes, allow
	// double the expected time per character for a fudge factor.
	thisTimeOuts.WriteTotalTimeoutMultiplier = 0;
	thisTimeOuts.WriteTotalTimeoutConstant = 1000;
	SetCommTimeouts(this->comportDevice, &thisTimeOuts);


	memset(&dcb, 0, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);

	GetCommState(this->comportDevice, &dcb);

	dcb.fBinary = TRUE;
	dcb.fParity = TRUE;

	if (this->BaudRates == "300")
		dcb.BaudRate = CBR_300;
	else if (this->BaudRates == "600")
		dcb.BaudRate = CBR_600;
	else if (this->BaudRates == "1200")
		dcb.BaudRate = CBR_1200;
	else if (this->BaudRates == "2400")
		dcb.BaudRate = CBR_2400;
	else if (this->BaudRates == "4800")
		dcb.BaudRate = CBR_4800;
	else if (this->BaudRates == "9600")
		dcb.BaudRate = CBR_9600;
	else if (this->BaudRates == "14400")
		dcb.BaudRate = CBR_14400;
	else if (this->BaudRates == "19200")
		dcb.BaudRate = CBR_19200;
	else if (this->BaudRates == "28800")
		dcb.BaudRate = CBR_38400;
	else if (this->BaudRates == "33600")
		dcb.BaudRate = CBR_38400;
	else if (this->BaudRates == "38400")
		dcb.BaudRate = CBR_38400;
	else if (this->BaudRates == "56000")
		dcb.BaudRate = CBR_56000;
	else if (this->BaudRates == "57600")
		dcb.BaudRate = CBR_57600;
	else if (this->BaudRates == "115200")
		dcb.BaudRate = CBR_115200;
	else if (this->BaudRates == "128000")
		dcb.BaudRate = CBR_128000;
	else if (this->BaudRates == "256000")
		dcb.BaudRate = CBR_256000;
	else if (this->BaudRates == "PCI_9600")
		dcb.BaudRate = 1075;
	else if (this->BaudRates == "PCI_19200")
		dcb.BaudRate = 2212;
	else if (this->BaudRates == "PCI_38400")
		dcb.BaudRate = 4300;
	else if (this->BaudRates == "PCI_57600")
		dcb.BaudRate = 6450;
	else if (this->BaudRates == "PCI_500K")
		dcb.BaudRate = 56000;


	if (this->Parity == "None")
		dcb.Parity = NOPARITY;
	else if (this->Parity == "Even")
		dcb.Parity = EVENPARITY;
	else if (this->Parity == "Odd")
		dcb.Parity = ODDPARITY;

	if (this->DataBits == "7 Bit")
		dcb.ByteSize = 7;
	else if (this->DataBits == "8 Bit")
		dcb.ByteSize = 8;

	if (this->StopBits == "1 Bit")
		dcb.StopBits = ONESTOPBIT;
	else if (this->StopBits == "1.5 Bit")
		dcb.StopBits = ONE5STOPBITS;
	else if (this->StopBits == "2 Bit")
		dcb.StopBits = TWOSTOPBITS;

	dcb.fRtsControl = RTS_CONTROL_ENABLE;
	dcb.fDtrControl = DTR_CONTROL_ENABLE;
	dcb.fOutxDsrFlow = FALSE;

	if (this->isFlowChecked) {
		dcb.fOutX = FALSE;
		dcb.fInX = FALSE;
		dcb.XonLim = 2048;
		dcb.XoffLim = 1024;
	}
	else {
		dcb.fOutxCtsFlow = TRUE;
		dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
	}

	SetCommState(this->comportDevice, &dcb);
	SetCommMask(this->comportDevice, EV_RXCHAR);

}

void Comport::Clear()
{
	PurgeComm(this->comportDevice, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	memset(this->inBuffer, 0, MAX_BUFFER * 2);
	this->length = 0;
}

void Comport::ComportReader()
{
	//extern short g_nRemoteStatus;
	DWORD       errorFlags;
	COMSTAT     comstat;
	DWORD	eventMask;
	char	buffer[MAX_BUFFER];
	DWORD	length;
	int	size;
	int	insize = 0;

	while (this->isOpened) 
	{
		eventMask = 0;
		length = 0;
		insize = 0;
		memset(buffer, '\0', MAX_BUFFER);
		WaitCommEvent(this->comportDevice, &eventMask, NULL);
		ClearCommError(this->comportDevice, &errorFlags, &comstat);
		if ((eventMask & EV_RXCHAR) && comstat.cbInQue) 
		{
			size = comstat.cbInQue > MAX_BUFFER ? MAX_BUFFER : comstat.cbInQue;

			do 
			{
				ClearCommError(this->comportDevice, &errorFlags, &comstat);
				if (!ReadFile(this->comportDevice, buffer + insize, size, &length, &(this->read))) 
				{
					// error
					TRACE("Error in ReadFile\n");
					if (GetLastError() == ERROR_IO_PENDING) 
					{
						if (WaitForSingleObject(this->read.hEvent, 1000) != WAIT_OBJECT_0)
						{
							length = 0;
						}
						else
						{
							GetOverlappedResult(this->comportDevice, &(this->read), &length, FALSE);
						}
					}
					else
					{
						length = 0;
					}
				}
				insize += length;
			} 
			while ((length != 0) && (insize < size));

			ClearCommError(this->comportDevice, &errorFlags, &comstat);

			if (this->length + insize > MAX_BUFFER * 2)
			{
				insize = (this->length + insize) - MAX_BUFFER * 2;
			}

			this->_event->ResetEvent();
			memcpy(this->inBuffer + this->length, buffer, insize);
			this->length += insize;
			this->_event->SetEvent();
			LPARAM temp = (LPARAM)this;
			
			__raise this->comportEvent.Received(this->inBuffer);


		}
	}
	PurgeComm(this->comportDevice, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	LPARAM temp = (LPARAM)this;
}

void ComportEvent::Received(char* bufer);