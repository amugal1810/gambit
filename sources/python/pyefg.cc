//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Python extension type for extensive form games
//
// This file is part of Gambit
// Copyright (c) 2003, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include <Python.h>
#include "game/efg.h"
#include "pygambit.h"

/*************************************************************************
 * EFG: TYPE DESCRIPTOR
 *************************************************************************/

staticforward void efg_dealloc(efgobject *);
staticforward PyObject *efg_getattr(efgobject *, char *);
staticforward int efg_compare(efgobject *, efgobject *);
staticforward int efg_print(efgobject *, FILE *, int); 

PyTypeObject Efgtype = {      /* main python type-descriptor */
  /* type header */                    /* shared by all instances */
  PyObject_HEAD_INIT(0)
  0,                               /* ob_size */
  "efg",                           /* tp_name */
  sizeof(efgobject),               /* tp_basicsize */
  0,                               /* tp_itemsize */

  /* standard methods */
  (destructor)  efg_dealloc,       /* tp_dealloc  ref-count==0  */
  (printfunc)   efg_print,         /* tp_print    "print x"     */
  (getattrfunc) efg_getattr,       /* tp_getattr  "x.attr"      */
  (setattrfunc) 0,                 /* tp_setattr  "x.attr=v"    */
  (cmpfunc)     efg_compare,       /* tp_compare  "x > y"       */
  (reprfunc)    0,                 /* tp_repr     `x`, print x  */

  /* type categories */
  0,                               /* tp_as_number   +,-,*,/,%,&,>>,pow...*/
  0,                               /* tp_as_sequence +,[i],[i:j],len, ...*/
  0,                               /* tp_as_mapping  [key], len, ...*/

  /* more methods */
  (hashfunc)     0,                /* tp_hash    "dict[x]" */
  (ternaryfunc)  0,                /* tp_call    "x()"     */
  (reprfunc)     0,                /* tp_str     "str(x)"  */
};  /* plus others: see Include/object.h */


/*****************************************************************************
 * INSTANCE METHODS
 *****************************************************************************/

static PyObject *
efg_getchance(efgobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  efplayerobject *player = newefplayerobject();
  *player->m_efplayer = self->m_efg->GetChance();
  return (PyObject *) player;
}

static PyObject *
efg_getlabel(efgobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("s", (char *) self->m_efg->GetTitle());
}

static PyObject *
efg_getoutcome(efgobject *self, PyObject *args)
{
  int index;

  if (!PyArg_ParseTuple(args, "i", &index)) {
    return NULL;
  }

  if (index < 1 || index > self->m_efg->NumOutcomes()) {
    return NULL;
  }

  efoutcomeobject *outcome = newefoutcomeobject();
  *outcome->m_efoutcome = self->m_efg->GetOutcome(index);
  return (PyObject *) outcome;
}

static PyObject *
efg_getplayer(efgobject *self, PyObject *args)
{
  int index;

  if (!PyArg_ParseTuple(args, "i", &index)) {
    return NULL;
  }

  if (index < 1 || index > self->m_efg->NumPlayers()) {
    return NULL;
  }

  efplayerobject *player = newefplayerobject();
  *player->m_efplayer = self->m_efg->GetPlayer(index);
  return (PyObject *) player;
}

static PyObject *
efg_getroot(efgobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  nodeobject *node = newnodeobject();
  *node->m_node = self->m_efg->RootNode();
  return (PyObject *) node;
}

static PyObject *
efg_numoutcomes(efgobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("i", self->m_efg->NumOutcomes());
}

static PyObject *
efg_numplayers(efgobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("i", self->m_efg->NumPlayers());
}

static PyObject *
efg_setlabel(efgobject *self, PyObject *args)
{
  char *label;

  if (!PyArg_ParseTuple(args, "s", &label)) {
    return NULL;
  }

  self->m_efg->SetTitle(label);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *
efg_writeefg(efgobject *self, PyObject *args)
{
  char *filename;

  if (!PyArg_ParseTuple(args, "s", &filename)) {
    return NULL;
  }

  try {
    gFileOutput file(filename);
    self->m_efg->WriteEfgFile(file, 6);
    Py_INCREF(Py_None);
    return Py_None;
  }
  catch (const gFileInput::OpenFailed &) {
    Py_INCREF(Py_None);
    return Py_None;
  }
  catch (...) {
    Py_INCREF(Py_None);
    return Py_None;
  }

  Py_INCREF(Py_None);
  return Py_None;
}

static struct PyMethodDef efg_methods[] = {
  { "GetChance", (PyCFunction) efg_getchance, 1 },
  { "GetLabel", (PyCFunction) efg_getlabel, 1 },
  { "GetOutcome", (PyCFunction) efg_getoutcome, 1 },
  { "GetPlayer", (PyCFunction) efg_getplayer, 1 },
  { "GetRoot", (PyCFunction) efg_getroot, 1 },
  { "NumOutcomes", (PyCFunction) efg_numoutcomes, 1 },
  { "NumPlayers", (PyCFunction) efg_numplayers, 1 },
  { "SetLabel", (PyCFunction) efg_setlabel, 1 },
  { "WriteEfg", (PyCFunction) efg_writeefg, 1 },
  { NULL, NULL }
};

/*****************************************************************************
 * BASIC TYPE-OPERATIONS
 *****************************************************************************/

efgobject *
newefgobject(void)
{
  efgobject *self;
  self = PyObject_NEW(efgobject, &Efgtype);
  if (self == NULL) {
    return NULL;
  }
  self->m_efg = 0;
  return self;
}

static void                   
efg_dealloc(efgobject *self) 
{                            
  PyMem_DEL(self);           
}

static PyObject *
efg_getattr(efgobject *self, char *name)
{
  return Py_FindMethod(efg_methods, (PyObject *) self, name);
}

static int
efg_compare(efgobject *obj1, efgobject *obj2)
{
  // Implementation: If games are the game underlying object, return
  // equal; otherwise, order by their Python pointer addresses.
  if (*obj1->m_efg == *obj2->m_efg) {
    return 0;
  }
  else if (obj1->m_efg < obj2->m_efg) {
    return -1;
  }
  else {
    return 1;
  }
}

static int
efg_print(efgobject *self, FILE *fp, int flags)
{
  fprintf(fp, "<{efg} \"%s\">", (char *) self->m_efg->GetTitle());
  return 0;
}

/************************************************************************
 * MODULE METHODS
 ************************************************************************/

PyObject *
gbt_read_efg(PyObject *self, PyObject *args)
{
  char *filename;

  if (!PyArg_ParseTuple(args, "s", &filename)) {
    return NULL;
  }

  efgobject *efg = newefgobject();
  try {
    gFileInput file(filename);
    efg->m_efg = new gbtEfgGame(ReadEfgFile(file));
    return (PyObject *) efg;
  }
  catch (const gFileInput::OpenFailed &) {
    Py_INCREF(Py_None);
    return Py_None;
  }
  catch (...) {
    Py_INCREF(Py_None);
    return Py_None;
  }
}

void
initefg(void)
{
  Efgtype.ob_type = &PyType_Type;
}