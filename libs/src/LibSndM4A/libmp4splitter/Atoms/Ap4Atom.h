#pragma once

/*****************************************************************
|
|    AP4 - Atoms 
|
|    Copyright 2002 Gilles Boccon-Gibod
|
|
|    This file is part of Bento4/AP4 (MP4 Atom Processing Library).
|
|    Unless you have obtained Bento4 under a difference license,
|    this version of Bento4 is Bento4|GPL.
|    Bento4|GPL is free software; you can redistribute it and/or modify
|    it under the terms of the GNU General Public License as published by
|    the Free Software Foundation; either version 2, or (at your option)
|    any later version.
|
|    Bento4|GPL is distributed in the hope that it will be useful,
|    but WITHOUT ANY WARRANTY; without even the implied warranty of
|    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|    GNU General Public License for more details.
|
|    You should have received a copy of the GNU General Public License
|    along with Bento4|GPL; see the file COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
 ****************************************************************/

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4List.h"
#include "Ap4ByteStream.h"
#include "Ap4Debug.h"

/*----------------------------------------------------------------------
|       macros
+---------------------------------------------------------------------*/
#define AP4_ATOM_TYPE(a,b,c,d)  \
   ((((unsigned long)a)<<24) |  \
    (((unsigned long)b)<<16) |  \
    (((unsigned long)c)<< 8) |  \
    (((unsigned long)d)    ))

/*----------------------------------------------------------------------
|       constants
+---------------------------------------------------------------------*/
const int AP4_ATOM_HEADER_SIZE      = 8;
const int AP4_FULL_ATOM_HEADER_SIZE = 12;
const int AP4_ATOM_MAX_NAME_SIZE    = 256;
const int AP4_ATOM_MAX_URI_SIZE     = 512;

/*----------------------------------------------------------------------
|       forward references
+---------------------------------------------------------------------*/
class AP4_AtomParent;

/*----------------------------------------------------------------------
|       AP4_AtomInspector
+---------------------------------------------------------------------*/
/*
class AP4_AtomInspector {
public:
    // types
    typedef enum {
        HINT_NONE,
        HINT_HEX,
        HINT_BOOLEAN
    } FormatHint;

    // constructor and destructor
    AP4_AtomInspector() {}
    virtual ~AP4_AtomInspector() {}

    // methods
    virtual void StartElement(const char* name, const char* extra = NULL) {}
    virtual void EndElement() {}
    virtual void AddField(const char* name, AP4_UI32 value, FormatHint hint = HINT_NONE) {}
    virtual void AddField(const char* name, const char* value, FormatHint hint = HINT_NONE) {}
};
*/

/*----------------------------------------------------------------------
|       AP4_Atom
+---------------------------------------------------------------------*/
class AP4_Atom {
 public:
    // types
    typedef AP4_UI32 Type;

    // methods
                       AP4_Atom(Type type, 
                                bool is_full = false);
                       AP4_Atom(Type     type, 
                                AP4_Size size, 
                                bool     is_full = false);
                       AP4_Atom(Type            type, 
                                AP4_Size        size, 
                                bool            is_full,
                                AP4_ByteStream& stream);
    virtual           ~AP4_Atom() {}
    Type               GetType() { return m_Type; }
    void               SetType(Type type) { m_Type = type; }
    AP4_Size           GetHeaderSize();
    virtual AP4_Size   GetSize() { return m_Size; }
/*
    virtual AP4_Result Write(AP4_ByteStream& stream);
    virtual AP4_Result WriteHeader(AP4_ByteStream& stream);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream) = 0;
*/
/*
    virtual AP4_Result Inspect(AP4_AtomInspector& inspector);
    virtual AP4_Result InspectHeader(AP4_AtomInspector& inspector);
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector) {
        return AP4_SUCCESS; 
    }
*/

    // parent/child realtionship methods
    virtual AP4_Result      SetParent(AP4_AtomParent* parent) {
        m_Parent = parent;
        return AP4_SUCCESS;
    }
    virtual AP4_AtomParent* GetParent() { return m_Parent; }
    virtual AP4_Result Detach();

    // override this if your want to make an atom cloneable
    virtual AP4_Atom*  Clone() { return NULL; }

 protected:
    // members
    Type            m_Type;
    AP4_Size        m_Size;
    bool            m_IsFull;
    AP4_UI32        m_Version;
    AP4_UI32        m_Flags;
    AP4_AtomParent* m_Parent;
};

/*----------------------------------------------------------------------
|       AP4_AtomParent
+---------------------------------------------------------------------*/
class AP4_AtomParent {
public:
    // base methods
    virtual ~AP4_AtomParent();
    AP4_List<AP4_Atom>& GetChildren() { return m_Children; }
    virtual AP4_Result  AddChild(AP4_Atom* child, int position = -1);
    virtual AP4_Result  RemoveChild(AP4_Atom* child);
    virtual AP4_Result  DeleteChild(AP4_Atom::Type type);
    virtual AP4_Atom*   GetChild(AP4_Atom::Type type, AP4_Ordinal index = 0);
    virtual AP4_Atom*   FindChild(const char* path, 
                                  bool        auto_create = false);

    // methods designed to be overridden
    virtual void OnChildChanged(AP4_Atom* child) {}
    virtual void OnChildAdded(AP4_Atom* child)   {}
    virtual void OnChildRemoved(AP4_Atom* child) {}

protected:
    // members
    AP4_List<AP4_Atom> m_Children;
};

/*----------------------------------------------------------------------
|       AP4_UnknownAtom
+---------------------------------------------------------------------*/
class AP4_UnknownAtom : public AP4_Atom {
public:
    // constructor and destructor
    AP4_UnknownAtom(AP4_Atom::Type   type, 
                    AP4_Size         size, 
                    bool             is_full,
                    AP4_ByteStream&  stream);
    ~AP4_UnknownAtom();

    // methods
//    virtual AP4_Result WriteFields(AP4_ByteStream& stream);

private:
    // members
    AP4_ByteStream* m_SourceStream;
    AP4_Offset      m_SourceOffset;
};

/*----------------------------------------------------------------------
|       atom types
+---------------------------------------------------------------------*/
const AP4_Atom::Type AP4_ATOM_TYPE_UDTA = AP4_ATOM_TYPE('u','d','t','a');
const AP4_Atom::Type AP4_ATOM_TYPE_URL  = AP4_ATOM_TYPE('u','r','l',' ');
const AP4_Atom::Type AP4_ATOM_TYPE_TRAK = AP4_ATOM_TYPE('t','r','a','k');
const AP4_Atom::Type AP4_ATOM_TYPE_TKHD = AP4_ATOM_TYPE('t','k','h','d');
const AP4_Atom::Type AP4_ATOM_TYPE_STTS = AP4_ATOM_TYPE('s','t','t','s');
const AP4_Atom::Type AP4_ATOM_TYPE_STSZ = AP4_ATOM_TYPE('s','t','s','z');
const AP4_Atom::Type AP4_ATOM_TYPE_STSS = AP4_ATOM_TYPE('s','t','s','s');
const AP4_Atom::Type AP4_ATOM_TYPE_STSD = AP4_ATOM_TYPE('s','t','s','d');
const AP4_Atom::Type AP4_ATOM_TYPE_STSC = AP4_ATOM_TYPE('s','t','s','c');
const AP4_Atom::Type AP4_ATOM_TYPE_STCO = AP4_ATOM_TYPE('s','t','c','o');
const AP4_Atom::Type AP4_ATOM_TYPE_STBL = AP4_ATOM_TYPE('s','t','b','l');
const AP4_Atom::Type AP4_ATOM_TYPE_SINF = AP4_ATOM_TYPE('s','i','n','f');
const AP4_Atom::Type AP4_ATOM_TYPE_SCHM = AP4_ATOM_TYPE('s','c','h','m');
const AP4_Atom::Type AP4_ATOM_TYPE_SCHI = AP4_ATOM_TYPE('s','c','h','i');
const AP4_Atom::Type AP4_ATOM_TYPE_MVHD = AP4_ATOM_TYPE('m','v','h','d');
const AP4_Atom::Type AP4_ATOM_TYPE_MP4S = AP4_ATOM_TYPE('m','p','4','s');
const AP4_Atom::Type AP4_ATOM_TYPE_MP4A = AP4_ATOM_TYPE('m','p','4','a');
const AP4_Atom::Type AP4_ATOM_TYPE_MP4V = AP4_ATOM_TYPE('m','p','4','v');
const AP4_Atom::Type AP4_ATOM_TYPE_AVC1 = AP4_ATOM_TYPE('a','v','c','1');
const AP4_Atom::Type AP4_ATOM_TYPE_ENCA = AP4_ATOM_TYPE('e','n','c','a');
const AP4_Atom::Type AP4_ATOM_TYPE_ENCV = AP4_ATOM_TYPE('e','n','c','v');
const AP4_Atom::Type AP4_ATOM_TYPE_MOOV = AP4_ATOM_TYPE('m','o','o','v');
const AP4_Atom::Type AP4_ATOM_TYPE_MINF = AP4_ATOM_TYPE('m','i','n','f');
const AP4_Atom::Type AP4_ATOM_TYPE_META = AP4_ATOM_TYPE('m','e','t','a');
const AP4_Atom::Type AP4_ATOM_TYPE_MDHD = AP4_ATOM_TYPE('m','d','h','d');
const AP4_Atom::Type AP4_ATOM_TYPE_ILST = AP4_ATOM_TYPE('i','l','s','t');
const AP4_Atom::Type AP4_ATOM_TYPE_HDLR = AP4_ATOM_TYPE('h','d','l','r');
const AP4_Atom::Type AP4_ATOM_TYPE_FTYP = AP4_ATOM_TYPE('f','t','y','p');
const AP4_Atom::Type AP4_ATOM_TYPE_ESDS = AP4_ATOM_TYPE('e','s','d','s');
const AP4_Atom::Type AP4_ATOM_TYPE_EDTS = AP4_ATOM_TYPE('e','d','t','s');
const AP4_Atom::Type AP4_ATOM_TYPE_DRMS = AP4_ATOM_TYPE('d','r','m','s');
const AP4_Atom::Type AP4_ATOM_TYPE_DREF = AP4_ATOM_TYPE('d','r','e','f');
const AP4_Atom::Type AP4_ATOM_TYPE_DINF = AP4_ATOM_TYPE('d','i','n','f');
const AP4_Atom::Type AP4_ATOM_TYPE_CTTS = AP4_ATOM_TYPE('c','t','t','s');
const AP4_Atom::Type AP4_ATOM_TYPE_MDIA = AP4_ATOM_TYPE('m','d','i','a');
const AP4_Atom::Type AP4_ATOM_TYPE_VMHD = AP4_ATOM_TYPE('v','m','h','d');
const AP4_Atom::Type AP4_ATOM_TYPE_SMHD = AP4_ATOM_TYPE('s','m','h','d');
const AP4_Atom::Type AP4_ATOM_TYPE_NMHD = AP4_ATOM_TYPE('n','m','h','d');
const AP4_Atom::Type AP4_ATOM_TYPE_HMHD = AP4_ATOM_TYPE('h','m','h','d');
const AP4_Atom::Type AP4_ATOM_TYPE_FRMA = AP4_ATOM_TYPE('f','r','m','a');
const AP4_Atom::Type AP4_ATOM_TYPE_MDAT = AP4_ATOM_TYPE('m','d','a','t');
const AP4_Atom::Type AP4_ATOM_TYPE_FREE = AP4_ATOM_TYPE('f','r','e','e');
const AP4_Atom::Type AP4_ATOM_TYPE_TIMS = AP4_ATOM_TYPE('t','i','m','s');
const AP4_Atom::Type AP4_ATOM_TYPE_RTP  = AP4_ATOM_TYPE('r','t','p',' ');
const AP4_Atom::Type AP4_ATOM_TYPE_HNTI = AP4_ATOM_TYPE('h','n','t','i');
const AP4_Atom::Type AP4_ATOM_TYPE_SDP  = AP4_ATOM_TYPE('s','d','p',' ');
const AP4_Atom::Type AP4_ATOM_TYPE_IKMS = AP4_ATOM_TYPE('i','K','M','S');
const AP4_Atom::Type AP4_ATOM_TYPE_ISFM = AP4_ATOM_TYPE('i','S','F','M');
const AP4_Atom::Type AP4_ATOM_TYPE_HINT = AP4_ATOM_TYPE('h','i','n','t');
const AP4_Atom::Type AP4_ATOM_TYPE_TREF = AP4_ATOM_TYPE('t','r','e','f');

#if 0
/*----------------------------------------------------------------------
|       AP4_AtomListInspector
+---------------------------------------------------------------------*/
class AP4_AtomListInspector : public AP4_List<AP4_Atom>::Item::Operator
{
 public:
    AP4_AtomListInspector(AP4_AtomInspector& inspector) :
        m_Inspector(inspector) {}
    AP4_Result Action(AP4_Atom* atom) const {
        atom->Inspect(m_Inspector);
        return AP4_SUCCESS;
    }

 private:
    AP4_AtomInspector& m_Inspector;
};
#endif

/*----------------------------------------------------------------------
|       AP4_AtomListWriter
+---------------------------------------------------------------------*/
/*
class AP4_AtomListWriter : public AP4_List<AP4_Atom>::Item::Operator
{
 public:
    AP4_AtomListWriter(AP4_ByteStream& stream) :
        m_Stream(stream) {}
    AP4_Result Action(AP4_Atom* atom) const {
        atom->Write(m_Stream);
        return AP4_SUCCESS;
    }

 private:
    AP4_ByteStream& m_Stream;
};
*/

/*----------------------------------------------------------------------
|       AP4_AtomFinder
+---------------------------------------------------------------------*/
class AP4_AtomFinder : public AP4_List<AP4_Atom>::Item::Finder
{
 public:
    AP4_AtomFinder(AP4_Atom::Type type, AP4_Ordinal index = 0) : 
       m_Type(type), m_Index(index) {}
    AP4_Result Test(AP4_Atom* atom) const {
        if (atom->GetType() == m_Type) {
            if (m_Index-- == 0) {
                return AP4_SUCCESS;
            } else {
                return AP4_FAILURE;
            }
        } else {
            return AP4_FAILURE;
        }
    }
 private:
    AP4_Atom::Type      m_Type;
    mutable AP4_Ordinal m_Index;
};

/*----------------------------------------------------------------------
|       AP4_AtomSizeAdder
+---------------------------------------------------------------------*/
class AP4_AtomSizeAdder : public AP4_List<AP4_Atom>::Item::Operator {
public:
    AP4_AtomSizeAdder(AP4_Size& size) : m_Size(size) {}

private:
    AP4_Result Action(AP4_Atom* atom) const {
        m_Size += atom->GetSize();
        return AP4_SUCCESS;
    }
    AP4_Size& m_Size;
};

