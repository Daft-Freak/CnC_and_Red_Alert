#include "mouse.h"
#include "iff.h"
#include "shape.h"

extern void *MainWindow;

static WWMouseClass *_Mouse = NULL;

WWMouseClass::WWMouseClass(GraphicViewPortClass *scr, int mouse_max_width, int mouse_max_height) : MaxWidth(mouse_max_width), MaxHeight(mouse_max_width), State(0)
{
    Set_Cursor_Clip();
    _Mouse = this;

    MouseCursor = new uint8_t[MaxWidth * MaxHeight];
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

    MouseXHot = xhotspot;
    MouseYHot = yhotspot;
    return old_cursor;
}

void WWMouseClass::Hide_Mouse(void)
{
    if(!State++)
    {}
}

void WWMouseClass::Show_Mouse(void)
{
    if(!State)
        return;
    if(--State == 0)
    {
        if(PaletteDirty)
            Update_Palette();
        
    }
}

void WWMouseClass::Conditional_Hide_Mouse(int x1, int y1, int x2, int y2)
{
    //printf("WMouseClass::%s(%i, %i, %i, %i)\n", __func__, x1, y1, x2, y2);
}

void WWMouseClass::Conditional_Show_Mouse(void)
{
    //printf("WWMouseClass:%s\n", __func__);
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
    // we're using a "hardware" cursor, so don't need to do anything
}

void WWMouseClass::Erase_Mouse(GraphicViewPortClass *scr, bool forced)
{

}

void WWMouseClass::Set_Cursor_Clip(void)
{
    
}

void WWMouseClass::Clear_Cursor_Clip(void)
{
    
}

void WWMouseClass::Update_Palette()
{
    if(!WindowBuffer | !SDLSurface)
        return;

    if(State)
    {
        // don't do anything now if cursor is hidden anyway
        PaletteDirty = true;
        return;
    }

    PaletteDirty = false;

}

void WWMouseClass::Update_Pos(int x, int y)
{
    LastX = x;
    LastY = y;
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