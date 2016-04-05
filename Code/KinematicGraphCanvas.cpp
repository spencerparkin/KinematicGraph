// KinematicGraphCanvas.cpp

#include "KinematicGraphCanvas.h"
#include "KinematicGraphApp.h"
#include "KinematicGraph.h"
#include <gl/GLU.h>

int KinematicGraphCanvas::attributeList[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, 0 };

// TODO: We might let the mouse wheel resize a selected edge.
//       I would do this by deleting the edge, moving each of the
//       vertices, then create a new edge.

KinematicGraphCanvas::KinematicGraphCanvas( wxWindow* parent ) : wxGLCanvas( parent, wxID_ANY, attributeList, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS )
{
	dragging = false;

	window.xMin = -5.f;
	window.xMax = 5.f;
	window.yMin = -5.f;
	window.yMax = 5.f;

	mouseLocation.set( c3ga::vectorE3GA::coord_e1_e2_e3, 0.f, 0.f, 0.f );

	context = nullptr;

	Bind( wxEVT_PAINT, &KinematicGraphCanvas::OnPaint, this );
	Bind( wxEVT_SIZE, &KinematicGraphCanvas::OnSize, this );
	Bind( wxEVT_LEFT_DOWN, &KinematicGraphCanvas::OnMouseLeftDown, this );
	Bind( wxEVT_LEFT_UP, &KinematicGraphCanvas::OnMouseLeftUp, this );
	Bind( wxEVT_MOTION, &KinematicGraphCanvas::OnMouseMotion, this );
	Bind( wxEVT_RIGHT_DOWN, &KinematicGraphCanvas::OnMouseRightDown, this );
	Bind( wxEVT_CHAR_HOOK, &KinematicGraphCanvas::OnCharHook, this );
}

/*virtual*/ KinematicGraphCanvas::~KinematicGraphCanvas( void )
{
	delete context;
}

void KinematicGraphCanvas::OnPaint( wxPaintEvent& event )
{
	Render( GL_RENDER );	
}

void KinematicGraphCanvas::OnCharHook( wxKeyEvent& event )
{
	KinematicGraph* kinematicGraph = wxGetApp().GetKinematicGraph();
	if( !kinematicGraph )
		return;

	wxChar key = event.GetKeyCode();
	switch( key )
	{
		case 'V':
		{
			wxPoint mousePos = event.GetPosition();
			mouseLocation = LocatePoint( mousePos );
			int id = kinematicGraph->InsertVertex( mouseLocation );
			kinematicGraph->SetSelectedId( id );
			Refresh();
			break;
		}
		case 'E':
		{
			wxPoint mousePos = event.GetPosition();
			int id = Render( GL_SELECT, &mousePos );
			kinematicGraph->ConnectVertices( kinematicGraph->GetSelectedId(), id );
			kinematicGraph->SetSelectedId( id );
			Refresh();
			break;
		}
		case 'S':
		{
			wxPoint mousePos = event.GetPosition();
			int id = Render( GL_SELECT, &mousePos );
			kinematicGraph->SetSelectedId( id );
			Refresh();
			break;
		}
	}
}

void KinematicGraphCanvas::OnMouseRightDown( wxMouseEvent& event )
{
	KinematicGraph* kinematicGraph = wxGetApp().GetKinematicGraph();
	if( kinematicGraph )
	{
		wxPoint mousePos = event.GetPosition();
		int id = Render( GL_SELECT, &mousePos );
		kinematicGraph->SetVertexStationary( id, !kinematicGraph->GetVertexStationary( id ) );
		Refresh();
	}
}

void KinematicGraphCanvas::OnMouseLeftDown( wxMouseEvent& event )
{
	KinematicGraph* kinematicGraph = wxGetApp().GetKinematicGraph();
	if( kinematicGraph )
	{
		wxPoint mousePos = event.GetPosition();
		int id = Render( GL_SELECT, &mousePos );
		kinematicGraph->SetSelectedId( id );
		dragging = true;
		Refresh();
	}
}

void KinematicGraphCanvas::OnMouseLeftUp( wxMouseEvent& event )
{
	KinematicGraph* kinematicGraph = wxGetApp().GetKinematicGraph();
	if( kinematicGraph )
	{
		kinematicGraph->SetSelectedId(0);
		dragging = false;
		Refresh();
	}
}

void KinematicGraphCanvas::OnMouseMotion( wxMouseEvent& event )
{
	mouseLocation = LocatePoint( event.GetPosition() );

	KinematicGraph* kinematicGraph = wxGetApp().GetKinematicGraph();
	if( kinematicGraph && dragging )
	{
		c3ga::vectorE3GA vertexLocation;
		int vertexId = kinematicGraph->GetSelectedId();
		if( kinematicGraph->GetVertexLocation( vertexId, vertexLocation ) )
		{
			c3ga::vectorE3GA delta = mouseLocation - vertexLocation;
			kinematicGraph->MoveVertex( vertexId, delta );
		}
	}

	Refresh();
}

void KinematicGraphCanvas::CalcAspectCorrectedWindow( Window& aspectCorrectedWindow )
{
	GLint viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );
	float viewportAspectRatio = float( viewport[2] ) / float( viewport[3] );

	float xDelta = window.xMax - window.xMin;
	float yDelta = window.yMax - window.yMin;
	float windowAspectRatio = xDelta / yDelta;

	aspectCorrectedWindow = window;

	if( viewportAspectRatio > windowAspectRatio )
	{
		float delta = 0.5f * ( viewportAspectRatio * yDelta - xDelta );
		aspectCorrectedWindow.xMin -= delta;
		aspectCorrectedWindow.xMax += delta;
	}
	else
	{
		float delta = 0.5f * ( xDelta / viewportAspectRatio - yDelta );
		aspectCorrectedWindow.yMin -= delta;
		aspectCorrectedWindow.yMax += delta;
	}
}

c3ga::vectorE3GA KinematicGraphCanvas::LocatePoint( const wxPoint& point )
{
	Window aspectCorrectedWindow;
	CalcAspectCorrectedWindow( aspectCorrectedWindow );

	GLint viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );

	float u = float( point.x - viewport[0] ) / float( viewport[2] );
	float v = 1.f - float( point.y - viewport[1] ) / float( viewport[3] );

	float xDelta = aspectCorrectedWindow.xMax - aspectCorrectedWindow.xMin;
	float yDelta = aspectCorrectedWindow.yMax - aspectCorrectedWindow.yMin;

	c3ga::vectorE3GA location;
	location.set_e1( aspectCorrectedWindow.xMin + xDelta * u );
	location.set_e2( aspectCorrectedWindow.yMin + yDelta * v );
	location.set_e3( 0.f );

	return location;
}

int KinematicGraphCanvas::Render( GLenum renderMode, const wxPoint* mousePos /*= nullptr*/ )
{
	BindContext();

	int hitId = 0;
	int hitBufferSize = 512;
	unsigned int* hitBuffer = nullptr;
	if( renderMode == GL_SELECT )
	{
		hitBuffer = new unsigned int[ hitBufferSize ];
		glSelectBuffer( hitBufferSize, hitBuffer );
		glRenderMode( GL_SELECT );
		glInitNames();
		glPushName(0);
	}

	glClearColor( 1.f, 1.f, 1.f, 1.f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	if( renderMode == GL_SELECT )
	{
		GLint viewport[4];
		glGetIntegerv( GL_VIEWPORT, viewport );
		gluPickMatrix( GLdouble( mousePos->x ), GLdouble( viewport[3] - mousePos->y - 1 ), 10.0, 10.0, viewport );
	}

	Window aspectCorrectedWindow;
	CalcAspectCorrectedWindow( aspectCorrectedWindow );
	gluOrtho2D( aspectCorrectedWindow.xMin, aspectCorrectedWindow.xMax,
				aspectCorrectedWindow.yMin, aspectCorrectedWindow.yMax );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	KinematicGraph* kinematicGraph = wxGetApp().GetKinematicGraph();
	if( kinematicGraph )
		kinematicGraph->Render( renderMode );

	if( renderMode == GL_RENDER )
	{
		glColor3f( 1.f, 0.f, 0.f );
		glLineWidth( 1.f );
		glBegin( GL_LINE_LOOP );
		glVertex3f( mouseLocation.get_e1() + 0.1f, mouseLocation.get_e2(), mouseLocation.get_e3() );
		glVertex3f( mouseLocation.get_e1(), mouseLocation.get_e2() + 0.1f, mouseLocation.get_e3() );
		glVertex3f( mouseLocation.get_e1() - 0.1f, mouseLocation.get_e2(), mouseLocation.get_e3() );
		glVertex3f( mouseLocation.get_e1(), mouseLocation.get_e2() - 0.1f, mouseLocation.get_e3() );
		glEnd();
	}

	glFlush();

	if( renderMode == GL_RENDER )
		SwapBuffers();
	else if( renderMode == GL_SELECT )
	{
		int hitCount = glRenderMode( GL_RENDER );
		float smallestZ = 999.f;
		unsigned int* hitRecord = hitBuffer;
		for( int hit = 0; hit < hitCount; hit++ )
		{
			unsigned int nameCount = hitRecord[0];
			wxASSERT( nameCount == 1 );
			if( nameCount == 1 )
			{
				int id = hitRecord[3];
				float minZ = float( hitRecord[1] ) / float( 0x7FFFFFFF );
				if( minZ < smallestZ )
				{
					smallestZ = minZ;
					hitId = id;
				}
			}

			hitRecord += 3 + nameCount;
		}

		delete[] hitBuffer;
	}

	return hitId;
}

void KinematicGraphCanvas::OnSize( wxSizeEvent& event )
{
	BindContext();

	wxSize size = event.GetSize();
	glViewport( 0, 0, size.GetWidth(), size.GetHeight() );

	Refresh();
}

void KinematicGraphCanvas::BindContext( void )
{
	if( !context )
		context = new wxGLContext( this );

	SetCurrent( *context );
}

// KinematicGraphCanvas.cpp