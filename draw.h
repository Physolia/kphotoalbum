#include <qpoint.h>
#ifndef DRAW_H
#define DRAW_H
class QMouseEvent;
class QPainter;

class Draw
{
public:
    Draw() {};
    void startDraw( QMouseEvent* );
    virtual void draw( QMouseEvent*, QPainter& );
protected:
    QPoint _startPos;
    QPoint _lastPos;
};

#endif /* DRAW_H */

