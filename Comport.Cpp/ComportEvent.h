#pragma once
#ifndef _COMPORTEVENT_H_
#define _COMPORTEVENT_H_

[event_source(native)]
class ComportEvent
{
	public:
		__event void Received(char* buffer);
};

#endif // !_COMPORTEVENT_H_
