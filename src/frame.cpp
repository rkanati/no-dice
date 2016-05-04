
#include "frame.hpp"

#include <GL/gl.h>
#include <GL/glu.h>

namespace nd {
  Frame Frame::draw (v2i dims, float time, float alpha) {
    if (dims.x < 1) dims.x = 1;
    if (dims.y < 1) dims.y = 1;

    glViewport (0, 0, dims.x, dims.y);
    glClear (GL_DEPTH_BUFFER_BIT);

    // background
    glPolygonMode (GL_FRONT, GL_FILL);

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();

    glDisable (GL_DEPTH_TEST);
    glDepthMask (GL_FALSE);

    glBegin (GL_QUADS);
      glColor3f (0.00, 0.00, 0.13); glVertex2f (-1, -1);
      glColor3f (0.00, 0.00, 0.13); glVertex2f ( 1, -1);
      glColor3f (0.13, 0.03, 0.08); glVertex2f ( 1,  1);
      glColor3f (0.13, 0.03, 0.08); glVertex2f (-1,  1);
    glEnd ();

    // foreground
    glPolygonMode (GL_FRONT, GL_LINE);

    glEnable (GL_DEPTH_TEST);
    glDepthMask (GL_TRUE);

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    double aspect = double (dims.x) / double (dims.y);
    gluPerspective (75.0 / aspect, aspect, 0.1, 1000.0);

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    gluLookAt (
      0, -32, 0,
      0, 0, 0,
      0, 0, 1
    );

    for (auto const& item : chunk_items) {
      glPushMatrix ();
      glRotatef (time * 50.f, 0, 0, 1);
      glTranslatef (item.offset.x, item.offset.y, item.offset.z);
      item.mesh->draw ();
      glPopMatrix ();
    }

    return Frame { std::move (chunk_items) };
  }
}

