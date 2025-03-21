#include <stdio.h>

#include "timer.h"

#include "pico/stdlib.h"

bool TimerSystemOn = false;

static repeating_timer_t PicoTimer;

static bool TimerCallback(repeating_timer_t *timer)
{
    ((WinTimerClass *)timer->user_data)->Update_Tick_Count();

    return true;
}

// TimerClass/CountDownTimerClass are mostly used by TD
// (RA has it's own impl)
TimerClass::TimerClass(BaseTimerEnum timer, bool start) : Accumulated(0), Started(0), TickType(timer)
{
	if(start && TimerSystemOn)
        Start();
}

long TimerClass::Set(long value, bool start)
{
	Started = 0;
	Accumulated = value;
	if(start)
        return Start();

	return Time();
}

long TimerClass::Start(void)
{
    if(!Started)
        Started = Get_Ticks() + 1;
    return Time();
}

long TimerClass::Time(void)
{
	if(Started)
    {
		long ticks = Get_Ticks();
		Accumulated += ticks - (Started-1);
		Started = ticks+1;
	}
	return Accumulated;
}

long TimerClass::Get_Ticks(void)
{
	if(WindowsTimer && TickType == BT_SYSTEM) // BT_USER seems unused
		return WindowsTimer->Get_System_Tick_Count();

	return 0;
}

CountDownTimerClass::CountDownTimerClass(BaseTimerEnum timer, long set, bool on) : TimerClass(timer, on)
{
	Set(set, on);
}

CountDownTimerClass::CountDownTimerClass(BaseTimerEnum timer, bool on) : TimerClass(timer, false), DelayTime(0)
{
	if(on)
        Start();
}

long CountDownTimerClass::Set(long value, bool start)
{
	DelayTime = value;
	TimerClass::Reset(start);
	return Time();
}

long CountDownTimerClass::Time(void)
{
	long ticks = DelayTime - TimerClass::Time();

	if(ticks < 0)
		ticks = 0;

	return ticks;
}

WinTimerClass::WinTimerClass(unsigned freq, bool partial) : SysTicks(0), UserTicks(0)
{
	add_repeating_timer_ms(1000 / freq, TimerCallback, this, &PicoTimer);
}

WinTimerClass::~WinTimerClass()
{
	cancel_repeating_timer(&PicoTimer);
}

void WinTimerClass::Update_Tick_Count(void)
{
    SysTicks++;
    UserTicks++;
}

unsigned WinTimerClass::Get_System_Tick_Count(void)
{
    return SysTicks;
}

uint32_t Get_Time_Ms()
{
    return to_ms_since_boot(get_absolute_time());
}