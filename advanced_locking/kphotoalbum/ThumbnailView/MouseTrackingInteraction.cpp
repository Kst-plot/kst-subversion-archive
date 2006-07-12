#include "MouseTrackingInteraction.h"
#include "ThumbnailWidget.h"

ThumbnailView::MouseTrackingInteraction::MouseTrackingInteraction( ThumbnailWidget* view )
    :_view( view )
{
}


void ThumbnailView::MouseTrackingInteraction::mouseMoveEvent( QMouseEvent* event )
{
    static QString lastFileNameUderCursor;
    QString fileName = _view->fileNameAtCoordinate( event->pos(), ViewportCoordinates );
    if ( fileName != lastFileNameUderCursor ) {
        emit _view->fileNameUnderCursorChanged( fileName );
        lastFileNameUderCursor = fileName;
    }
}
