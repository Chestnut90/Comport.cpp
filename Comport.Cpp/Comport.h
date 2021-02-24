#pragma once
#ifndef _COMPORT_H_
#define _COMPORT_H_

#include "pch.h"
#include <thread>	// standard thread

#define	MAX_BUFFER		50000
#define	IN_BUFFER_SIZE	50000
#define	OUT_BUFFER_SIZE	50000
#define ASCII_XON		0x11
#define ASCII_XOFF		0x13
#define	WM_COM_RECEIVE	(WM_USER+1)	// Receiver
#define	WM_COM_CLOSE	(WM_USER+2)	// closer

class Comport
{
	public:
		//@ Constructor
		Comport(CString comportName, CString baudRates, CString parity, CString dataBits, CString stopBits);

		//@ Constructor
		Comport();

		//@ Destructor
		~Comport();

		//@ Is Comport Opened
		bool IsOpened();

		//@ Open Comport
		bool Open();

		//@ Close Comport
		void Close();

		//@ Receive
		int Receive(char* inBuffer, int len);

		//@ Send
		bool Send(const char* outBuffer, int len);

		//@ Create
		bool Create(HWND hWnd);

		//@ Handle close with safe
		void HandleClose();

		//@ Reset serial
		//@ study needed
		void ResetSerial();

		//@ clear
		void Clear();

		//@ Comport Threading
		void ComportReader();

	protected:

	private:

	// Members
	public:
		
		//@
		CString ComportName;
		
		//@
		CString BaudRates;

		//@
		CString Parity;

		//@
		CString DataBits;

		//@
		CString StopBits;

	private:

		//@ is comport opened
		bool isOpened = false;

		//@
		bool isFlowChecked = false;

		//@
		OVERLAPPED read, write;

		//@ handle
		HANDLE comportDevice;

		//@
		HWND hWnd;

		//@
		CEvent* _event;
		
		//@ buffer
		char inBuffer[MAX_BUFFER * 2];

		//@ length
		int length;

		//@ reading thread
		std::thread reader;

		//@ event
		ComportEvent comportEvent;
};



#endif // !_COMPORT_H_
