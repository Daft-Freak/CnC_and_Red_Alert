#include "mouse.h"
#include "iff.h"
#include "shape.h"

extern void *MainWindow;

static WWMouseClass *_Mouse = NULL;

WWMouseClass::WWMouseClass(GraphicViewPortClass *scr, int mouse_max_width, int mouse_max_height) : MaxWidth(mouse_max_width), MaxHeight(mouse_max_width),
    MCFlags(0), MCCount(0), EraseBuffX(-100), EraseBuffY(-100), Screen(scr), State(0)
{
    Set_Cursor_Clip();
    _Mouse = this;

    MouseCursor = new uint8_t[MaxWidth * MaxHeight * 2];
}

WWMouseClass::~WWMouseClass()
{
    Clear_Cursor_Clip();

    delete[] MouseCursor;
}

void *WWMouseClass::Set_Cursor(int xhotspot, int yhotspot, void *cursor)
{
    if(!cursor || PrevCursor == cursor)
        return cursor;


    auto cursor_shape = (Shape_Type *)cursor;

    int datasize = cursor_shape->DataLength;

    if(cursor_shape->Width > MaxWidth || cursor_shape->OriginalHeight > MaxHeight)
        return PrevCursor;

    // don't handle 16-color or uncompressed
    if(cursor_shape->ShapeType != 0)
    {
        printf("Set_Cursor type %i\n", cursor_shape->ShapeType);
        return PrevCursor;
    }

    // decompress it
    auto decompressed_data = new uint8_t[cursor_shape->DataLength];
    LCW_Uncompress((uint8_t *)cursor + 10, decompressed_data, cursor_shape->DataLength);

    // now we have an uncmpressed, but still encoded shape
    auto inptr = decompressed_data;

    int remaining = cursor_shape->Width * cursor_shape->OriginalHeight;
    auto outptr = MouseCursor;

    do
    {
        uint8_t pixel = *inptr++;
        if(pixel)
        {
            *outptr++ = pixel;
            remaining--;
        }
        else
        {
            // run of zeros
            int count = *inptr++;
            remaining -= count;
            while(count--)
            {
                *outptr++ = 0;
            }
        }
    }
    while(remaining);

    delete[] decompressed_data;

    // set it and clean up
    auto old_cursor = PrevCursor;
    

    PrevCursor = (char *)cursor;

    CursorWidth = cursor_shape->Width;
    CursorHeight = cursor_shape->OriginalHeight;
    MouseXHot = xhotspot;
    MouseYHot = yhotspot;
    return old_cursor;
}

void WWMouseClass::Hide_Mouse(void)
{
    if(!State++)
    {
        // erase mouse
        Erase_Mouse(Screen, true);
    }
}

void WWMouseClass::Show_Mouse(void)
{
    if(!State)
        return;
    if(--State == 0)
    {
        // draw it if not hidden
        Draw_Mouse(Screen);
    }
}

void WWMouseClass::Conditional_Hide_Mouse(int x1, int y1, int x2, int y2)
{

	//
	// First of all, adjust all the coordinates so that they handle
	// the fact that the hotspot is not necessarily the upper left
	// corner of the mouse.
	//
	x1 -= (CursorWidth - MouseXHot);
	x1  = MAX(0, x1);
	y1 -= (CursorHeight - MouseYHot);
	y1  = MAX(0, y1);
	x2  += MouseXHot;
	x2  = MIN(x2, Screen->Get_Width());
	y2  += MouseYHot;
	y2  = MIN(y2, Screen->Get_Height());

	// The mouse could be in one of four conditions.
	// 1) The mouse is visible and no conditional hide has been specified.
	// 	(perform normal region checking with possible hide)
	// 2) The mouse is hidden and no conditional hide as been specified.
	// 	(record region and do nothing)
	// 3) The mouse is visible and a conditional region has been specified
	// 	(expand region and perform check with possible hide).
	// 4) The mouse is already hidden by a previous conditional.
	// 	(expand region and do nothing)
	//
	// First: Set or expand the region according to the specified parameters
	if (!MCCount) {
		MouseCXLeft		= x1;
		MouseCYUpper	= y1;
		MouseCXRight	= x2;
		MouseCYLower	= y2;
	} else {
		MouseCXLeft		= MIN(x1, MouseCXLeft);
		MouseCYUpper	= MIN(y1, MouseCYUpper);
		MouseCXRight	= MAX(x2, MouseCXRight);
		MouseCYLower	= MAX(y2, MouseCYLower);
	}
	//
	// If the mouse isn't already hidden, then check its location against
	// the hiding region and hide if necessary.
	//
	if (!(MCFlags & CONDHIDDEN)) {

		if (LastX >= MouseCXLeft && LastX <= MouseCXRight && LastY >= MouseCYUpper && LastY <= MouseCYLower) {
			Hide_Mouse();
			MCFlags |= CONDHIDDEN;
		}
	}
	//
	// Record the fact that a conditional hide was called and then exit
	//
	//
	MCFlags |= CONDHIDE;
	MCCount++;
    
}

void WWMouseClass::Conditional_Show_Mouse(void)
{
	//
	// if there are any nested hides then dec the count
	//
	if (MCCount) {
		MCCount--;
		//
		// If the mouse is now not hidden and it had actually been
		// hidden before then display it.
		//
		if (!MCCount) {
            bool was_hidden = MCFlags & CONDHIDDEN;
            MCFlags = 0;

			if(was_hidden)
				Show_Mouse();
		}
	}
}

int WWMouseClass::Get_Mouse_State(void)
{
    return State;
}

int WWMouseClass::Get_Mouse_X(void)
{
    return LastX;
}

int WWMouseClass::Get_Mouse_Y(void)
{
    return LastY;
}

void WWMouseClass::Draw_Mouse(GraphicViewPortClass *scr)
{
    // do nothing if hidden
    if(State || (MCFlags & CONDHIDDEN))
        return;

    auto cursor_buf = MouseCursor;
    auto erase_buf = MouseCursor + MaxWidth * MaxHeight;

    // clipping
    int dx = LastX - MouseXHot;
    int dy = LastY - MouseYHot;
    int sx = 0, sy = 0;
    int w = CursorWidth;
    int h = CursorHeight;

    if(dx < 0)
    {
        sx = -dx;
        w -= dx;
        dx = 0;
    }
    else if(dx + w > scr->Get_Width())
        w = scr->Get_Width() - dx;

    if(dy < 0)
    {
        sy = -dy;
        h -= dy;
        dy = 0;
    }
    else if(dy + h > scr->Get_Height())
        h = scr->Get_Height() - dy;

    // draw and save existing for erase
    auto in_ptr = cursor_buf + sx + sy * CursorWidth;
    auto out_save = erase_buf + sx + sy * CursorWidth;

    if(!scr->Lock())
        return;

    int out_line = scr->Get_Width() + scr->Get_XAdd() + scr->Get_Pitch();
    auto out_ptr = scr->Get_Offset() + dx + dy * out_line;

    for(int y = 0; y < h; y++)
    {
        for(int x = 0; x < w; x++)
        {
            out_save[x] = out_ptr[x];

            if(in_ptr[x])
                out_ptr[x] = in_ptr[x];
        }

        in_ptr += CursorWidth;
        out_save += CursorWidth;
        out_ptr += out_line;
    }

    scr->Unlock();

    EraseBuffX = LastX - MouseXHot;
    EraseBuffY = LastY - MouseYHot;
}

void WWMouseClass::Erase_Mouse(GraphicViewPortClass *scr, bool forced)
{
    if(!forced)
        return;

    if(EraseBuffX != -100 || EraseBuffY != -100)
    {
        auto erase_buf = MouseCursor + MaxWidth * MaxHeight;

        // clipping
        int dx = EraseBuffX;
        int dy = EraseBuffY;
        int sx = 0, sy = 0;
        int w = CursorWidth;
        int h = CursorHeight;
    
        if(dx < 0)
        {
            sx = -dx;
            w -= dx;
            dx = 0;
        }
        else if(dx + w > scr->Get_Width())
            w = scr->Get_Width() - dx;
    
        if(dy < 0)
        {
            sy = -dy;
            h -= dy;
            dy = 0;
        }
        else if(dy + h > scr->Get_Height())
            h = scr->Get_Height() - dy;
    
        // restore saved
        auto in_save = erase_buf + sx + sy * CursorWidth;
    
        if(!scr->Lock())
            return;
    
        int out_line = scr->Get_Width() + scr->Get_XAdd() + scr->Get_Pitch();
        auto out_ptr = scr->Get_Offset() + dx + dy * out_line;
    
        for(int y = 0; y < h; y++)
        {
            for(int x = 0; x < w; x++)
                out_ptr[x] = in_save[x];
    
            in_save += CursorWidth;
            out_ptr += out_line;
        }
    
        scr->Unlock();
    
        EraseBuffX = -100;
        EraseBuffY = -100;
    }
}

void WWMouseClass::Set_Cursor_Clip(void)
{   
}

void WWMouseClass::Clear_Cursor_Clip(void)
{   
}

void WWMouseClass::Update_Palette()
{
}

void WWMouseClass::Update_Pos(int x, int y)
{
    if(LastX == x && LastY == y)
        return;

    LastX = x;
    LastY = y;

    // cursor hidden, don't need to update
    if(State)
        return;

    // erase old mouse
    Hide_Mouse();

    // check if it entered the hidden area
    if((MCFlags & CONDHIDE) && LastX >= MouseCXLeft && LastX <= MouseCXRight && LastY >= MouseCYUpper && LastY <= MouseCYLower)
        MCFlags |= CONDHIDDEN;

    // redraw if not hidden
    if(!(MCFlags & CONDHIDDEN))
        Show_Mouse();
}

void Hide_Mouse(void)
{
    if(_Mouse)
       _Mouse->Hide_Mouse();
}

void Show_Mouse(void)
{
    if(_Mouse)
       _Mouse->Show_Mouse();
}

void Conditional_Hide_Mouse(int x1, int y1, int x2, int y2)
{
    if(_Mouse)
       _Mouse->Conditional_Hide_Mouse(x1, y1, x2, y2);
}

void Conditional_Show_Mouse(void)
{
    if(_Mouse)
       _Mouse->Conditional_Show_Mouse();
}

int Get_Mouse_State(void)
{
    if(_Mouse)
        return _Mouse->Get_Mouse_State();
    return 0;
}

void *Set_Mouse_Cursor(int hotx, int hoty, void *cursor)
{
    if(_Mouse)
        return _Mouse->Set_Cursor(hotx, hoty, cursor);
    return 0;
}

int Get_Mouse_X(void)
{
    if(_Mouse)
        return _Mouse->Get_Mouse_X();
    return 0;
}

int Get_Mouse_Y(void)
{
    if(_Mouse)
        return _Mouse->Get_Mouse_Y();
    return 0;
}

void Update_Mouse_Palette()
{
    if(_Mouse)
        _Mouse->Update_Palette();
}

void Update_Mouse_Pos(int x, int y)
{
    if(_Mouse)
        _Mouse->Update_Pos(x, y);
}