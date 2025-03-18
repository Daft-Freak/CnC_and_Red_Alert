#include <string.h>

#include "keyboard.h"
#include "mouse.h"
#include "ww_win.h"

WWKeyboardClass::WWKeyboardClass() : MouseQX(0), MouseQY(0), Head(0), Tail(0)
{
    // clear buffer
    memset(Buffer, 0, 256);
}

bool WWKeyboardClass::Check(void)
{
    // poll for events, return key if any pressed
    //SDL_Event_Loop();

    if(Head == Tail) return false;

    return Buffer[Head];
}

int WWKeyboardClass::Get(void)
{
	while(!Check()) {}								// wait for key in buffer
	return Buff_Get();
}

bool WWKeyboardClass::Put(int key)
{
	int	temp = (Tail + 1) & 255;
	if(temp != Head)
	{
		Buffer[Tail] = (short)key;

		Tail = temp;
		return true;
	}
	return false;
}

bool WWKeyboardClass::Put_Key_Message(unsigned vk_key, bool release)
{
	//
	// Get the status of all of the different keyboard modifiers.  Note, only pay attention
	// to numlock and caps lock if we are dealing with a key that is affected by them.  Note
	// that we do not want to set the shift, ctrl and alt bits for Mouse keypresses as this
	// would be incompatible with the dos version.
	//
	if (vk_key != VK_LBUTTON && vk_key != VK_MBUTTON && vk_key != VK_RBUTTON)
    {
        
	}
	if (release)
        vk_key |= WWKEY_RLS_BIT;

	//
	// Finally use the put command to enter the key into the keyboard
	// system.
	//
	return Put(vk_key);
}

int WWKeyboardClass::To_ASCII(int num)
{
    if(num & WWKEY_RLS_BIT)
        return 0;

    return 0;
}

void WWKeyboardClass::Clear(void)
{
    Head = Tail;
}

int WWKeyboardClass::Down(int key)
{
    return 0;
}

bool WWKeyboardClass::Is_Mouse_Key(int key)
{
	key &= 0xFF;
	return key == VK_LBUTTON || key == VK_MBUTTON || key == VK_RBUTTON;
}

int WWKeyboardClass::Buff_Get(void)
{
	while (!Check()) {}								    // wait for key in buffer
	int temp = Buffer[Head];						    // get key out of the buffer
	int newhead = Head;									// save off head for manipulation
	if (Is_Mouse_Key(temp))
    {								// if key is a mouse then
		MouseQX	= Buffer[(Head + 1) & 255];			//		get the x and y pos
		MouseQY	= Buffer[(Head + 2) & 255];			//		from the buffer
		newhead += 3;		  									//		adjust head forward
	}
    else
		newhead += 1;		  									//		adjust head forward
	
	newhead	&= 255;
	Head		 = newhead;
	return temp ;
}