/*
    Copyright (c) 2014 Randy Gaul http://RandyGaul.net

    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:
      1. The origin of this software must not be misrepresented; you must not
         claim that you wrote the original software. If you use this software
         in a product, an acknowledgment in the product documentation would be
         appreciated but is not required.
      2. Altered source versions must be plainly marked as such, and must not be
         misrepresented as being the original software.
      3. This notice may not be removed or altered from any source distribution.
*/

#define GLEW_STATIC
#include <gl/glew.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "freeglut.h"

#include <cstring>
#include <vector>

#include "IEMath.h"

#define ESC_KEY 27

struct Poly
{
  std::vector<Vec2> vertices;
};

std::vector<Vec2> g_lines;
std::vector<Poly> g_polygons;

int lineState;

void BuildQuad( void );

void Keyboard( unsigned char key, int x, int y )
{
  switch(key)
  {
  case ESC_KEY:
    exit( 0 );
    break;
  case 'r':
    g_lines.clear( );
    g_polygons.clear( );
    lineState = 0;
    BuildQuad( );
    break;
  }
}

void RenderString( int x, int y, const char *s )
{
  glColor3f( 0.5f, 0.5f, 0.9f );
  glRasterPos2i( x, y );
  unsigned l = (unsigned)std::strlen( s );
  for(unsigned i = 0; i < l; ++i)
    glutBitmapCharacter( GLUT_BITMAP_9_BY_15, *(s + i) );
}

void Mouse( int button, int state, int x, int y )
{
  f32 xf = (f32)x * (1.0f / 10.0f);
  f32 yf = (f32)y * (1.0f / 10.0f);

  if(state == GLUT_DOWN)
  {
    switch(button)
    {
    case GLUT_LEFT_BUTTON:
      {
        g_lines.push_back( Vec2( xf, yf ) );
        ++lineState;
      } break;
    case GLUT_RIGHT_BUTTON:
      {
      } break;
    }
  }
}

// Point to plane classification for intersection of half-space
#define InFront( a ) \
  ((d##a) > EPSILON)

// Point to plane classification for non-intersection of half-space
#define Behind( a ) \
  ((d##a) < -EPSILON)

// Point to plane classification for laying on half-space horizon
#define On( a ) \
  (!InFront( a ) && !Behind( a ))

// Compute intersection point of a line segment and plane
Vec2 Intersect( const Vec2& a, const Vec2& b, f32 da, f32 db )
{
  return a + (da / (da - db)) * (b - a);
}

// Splits a polygon in half along a splitting plane using a clipping algorithm
// call Sutherland-Hodgman clipping
// Resource: Page 367 of Ericson (Real-Time Collision Detection)
void SutherlandHodgman( const Vec2& n, f32 d, const Poly *poly, std::vector<Poly> *out )
{
  Poly frontPoly;
  Poly backPoly;
  uint32 s = poly->vertices.size( );

  Vec2 a = poly->vertices[s - 1];
  f32 da = Dot( n, a ) - d;

  for(uint32 i = 0; i < s; ++i)
  {
    Vec2 b = poly->vertices[i];
    f32 db = Dot( n, b ) - d;

    if(InFront( b ))
    {
      if(Behind( a ))
      {
        Vec2 i = Intersect( b, a, db, da );
        real di = Dot( n, i ) - d;
        assert( !((di) > EPSILON) && !((di) < -EPSILON) );
        frontPoly.vertices.push_back( i );
        backPoly.vertices.push_back( i );
      }

      frontPoly.vertices.push_back( b );
    }
    else if(Behind( b ))
    {
      if(InFront( a ))
      {
        Vec2 i = Intersect( a, b, da, db );
        real di = Dot( n, i ) - d;
        assert( !((di) > EPSILON) && !((di) < -EPSILON) );
        frontPoly.vertices.push_back( i );
        backPoly.vertices.push_back( i );
      }
      else if(On( a ))
        backPoly.vertices.push_back( a );

      backPoly.vertices.push_back( b );
    }
    else
    {
      frontPoly.vertices.push_back( b );

      if(On( a ))
        backPoly.vertices.push_back( b );
    }

    a = b;
    da = db;
  }

  if(frontPoly.vertices.size( ))
    out->push_back( frontPoly );
  if(backPoly.vertices.size( ))
    out->push_back( backPoly );
}

// n - The normal of the clipping plane
// d - Distance of clipping plane from the origin
void SliceAllPolygons( const Vec2& n, f32 d )
{
  std::vector<Poly> out;

  for(uint32 i = 0; i < g_polygons.size( ); ++i)
    SutherlandHodgman( n, d, &g_polygons[i], &out );

  g_polygons = out;
}

// Polygons can be triangulated easily due to convexity
void TriangleFan( Poly *poly )
{
  std::vector<Vec2>& v = poly->vertices;

  Vec2 c = v[0];

  for(uint32 ia = 1, ib = 2; ib < v.size( ); ia = ib, ++ib)
  {
    Vec2 a = v[ia];
    Vec2 b = v[ib];
    
    glColor3f( Random( 0.1f, 1.0f ), Random( 0.1f, 1.0f ), Random( 0.1f, 1.0f ) );
    glBegin( GL_TRIANGLES );
      glVertex2f( a.x, a.y );
      glVertex2f( b.x, b.y );
      glVertex2f( c.x, c.y );
    glEnd( );
  }
}

void MainLoop( void )
{
  // Check to see if two new points were added to the line buffer
  // Clip all existing triangles against the new plane (2d line)
  if(lineState == 2)
  {
    lineState = 0;
    uint32 s = g_lines.size( );
    Vec2 n = g_lines[s - 1] - g_lines[s - 2];
    f32 l = n.Len( );

    // Try to keep visible lines a decent length
    if(l < 10.0f)
    {
      real factor = 5.0f / l;
      Vec2 c = g_lines[s - 1] - g_lines[s - 2];
      c * 0.5f;
      c += g_lines[s - 2];
      g_lines[s - 1] = c + n * factor;
      g_lines[s - 2] = c - n * factor;
    }

    n.Normalize( );
    n.Set( -n.y, n.x );
    f32 d = Dot( g_lines[s - 1], n );

    SliceAllPolygons( n, d );
  }

  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  srand( 1 );
  for(uint32 i = 0; i < g_polygons.size( ); ++i)
    TriangleFan( &g_polygons[i] );

  glColor3f( 1.0f, 1.0f, 1.0f );
  glBegin( GL_LINES );
  for(uint32 i = 0; i < g_lines.size( ); ++i)
    glVertex2f( g_lines[i].x, g_lines[i].y );
  glEnd( );

  RenderString( 1, 2, "Click once on either side of the square to slice" );
  RenderString( 1, 4, "Press r to reset" );

  glutSwapBuffers( );
}

void BuildQuad( void )
{
  Poly quad;

  quad.vertices.push_back( Vec2( -10.0f + 40.0f, -10.0f + 30.0f ) );
  quad.vertices.push_back( Vec2( -10.0f + 40.0f,  10.0f + 30.0f ) );
  quad.vertices.push_back( Vec2(  10.0f + 40.0f,  10.0f + 30.0f ) );
  quad.vertices.push_back( Vec2(  10.0f + 40.0f, -10.0f + 30.0f ) );

  g_polygons.push_back( quad );
}

int main( int argc, char **argv )
{
  const int WindowWidth = 800;
  const int WindowHeight = 600;
  glutInit( &argc, argv );
  int screenWidth = glutGet( GLUT_SCREEN_WIDTH );
  int screneHeight = glutGet( GLUT_SCREEN_HEIGHT );
  glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA );
  glutInitWindowSize( WindowWidth, WindowHeight );
  glutInitWindowPosition( (screenWidth - WindowWidth) / 2, (screneHeight - WindowHeight) / 2 );
  glutCreateWindow( "ShapeSlice Demo by Randy Gaul" );
  glewInit( );

  glutDisplayFunc( MainLoop );
  glutKeyboardFunc( Keyboard );
  glutIdleFunc( MainLoop );
  glutMouseFunc( Mouse );

  glMatrixMode( GL_PROJECTION );
  gluOrtho2D( 0, 80, 60, 0 );

  glutShowWindow( );

  BuildQuad( );

  glutMainLoop( );
}
