// KinematicGraphCanvas.h

#pragma once

#include <wx/glcanvas.h>
#include "C3GA/c3ga.h"

class KinematicGraphCanvas : public wxGLCanvas
{
public:

	KinematicGraphCanvas( wxWindow* parent );
	virtual ~KinematicGraphCanvas( void );

	void OnPaint( wxPaintEvent& event );
	void OnSize( wxSizeEvent& event );
	void OnMouseLeftDown( wxMouseEvent& event );
	void OnMouseLeftUp( wxMouseEvent& event );
	void OnMouseMotion( wxMouseEvent& event );
	void OnMouseRightDown( wxMouseEvent& event );
	void OnCharHook( wxKeyEvent& event );

private:

	void BindContext( void );
	int Render( GLenum renderMode, const wxPoint* mousePos = nullptr );

	struct Window
	{
		float xMin, xMax;
		float yMin, yMax;
	};

	void CalcAspectCorrectedWindow( Window& aspectCorrectedWindow );
	c3ga::vectorE3GA LocatePoint( const wxPoint& point );

	c3ga::vectorE3GA mouseLocation;
	Window window;
	bool dragging;
	wxGLContext* context;
	static int attributeList[];
};

// KinematicGraphCanvas.h