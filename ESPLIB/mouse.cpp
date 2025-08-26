#include "mouse.h"
#include "iff.h"
#include "shape.h"

extern void *MainWindow;

static WWMouseClass *_Mouse = NULL;

static uint8_t CursorDecompressBuffer[24 * 30]; // generally smaller

WWMouseClass::WWMouseClass(GraphicViewPortClass *scr, int mouse_max_width, int mouse_max_height) : MaxWidth(mouse_max_width), MaxHeight(mouse_max_width),
    MCFlags(0), MCCount(0), EraseBuffX(-100), EraseBuffY(-100), Screen(scr), PrevCursor(NULL), State(0)
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
    auto decompressed_data = CursorDecompressBuffer;
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

    // set it and clean up
    auto old_cursor = PrevCursor;

    PrevCursor = (char *)cursor;

    CursorWidth = cursor_shape->Width;
    CursorHeight = cursor_shape->OriginalHeight;
    MouseXHot = xhotspot;
    MouseYHot = yhotspot;

    /*display_set_cursor(MouseCursor, CursorWidth, CursorHeight);

    if(State == 0)
        display_set_cursor_pos(LastX - MouseXHot, LastY - MouseYHot);
    */

    return old_cursor;
}

void WWMouseClass::Hide_Mouse(void)
{
    if(!State++)
    {
        // move off screen
        //display_set_cursor_pos(0, 200);
    }
}

void WWMouseClass::Show_Mouse(void)
{
    if(!State)
        return;
    if(--State == 0)
    {
        // move back onto screen
        //display_set_cursor_pos(LastX - MouseXHot, LastY - MouseYHot);
    }
}

void WWMouseClass::Conditional_Hide_Mouse(int x1, int y1, int x2, int y2)
{
}

void WWMouseClass::Conditional_Show_Mouse(void)
{
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
}

void WWMouseClass::Update_Pos(int x, int y)
{
    if(LastX == x && LastY == y)
        return;

    // cursor hidden, don't need to update
    if(State)
    {
        LastX = x;
        LastY = y;
    
        return;
    }

    LastX = x;
    LastY = y;

    //display_set_cursor_pos(LastX - MouseXHot, LastY - MouseYHot);
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