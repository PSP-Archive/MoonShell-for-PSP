
LibName=LibSndM4A

include ../makefile_before.mak

# ----------------------------------------------------------------------------------
OBJS+= $(LibName).o
intlib_faad=libfaad2-2.7
OBJS+= $(intlib_faad)/bits.o $(intlib_faad)/cfft.o $(intlib_faad)/common.o $(intlib_faad)/decoder.o $(intlib_faad)/drc.o $(intlib_faad)/drm_dec.o $(intlib_faad)/error.o $(intlib_faad)/filtbank.o $(intlib_faad)/hcr.o $(intlib_faad)/huffman.o $(intlib_faad)/ic_predict.o $(intlib_faad)/is.o $(intlib_faad)/lt_predict.o $(intlib_faad)/_libfaad2-2.7_mdct.o $(intlib_faad)/mp4.o $(intlib_faad)/ms.o $(intlib_faad)/output.o $(intlib_faad)/pns.o $(intlib_faad)/ps_dec.o $(intlib_faad)/ps_syntax.o $(intlib_faad)/pulse.o $(intlib_faad)/rvlc.o $(intlib_faad)/sbr_dct.o $(intlib_faad)/sbr_dec.o $(intlib_faad)/sbr_e_nf.o $(intlib_faad)/sbr_fbt.o $(intlib_faad)/sbr_hfadj.o $(intlib_faad)/sbr_hfgen.o $(intlib_faad)/sbr_huff.o $(intlib_faad)/sbr_qmf.o $(intlib_faad)/sbr_syntax.o $(intlib_faad)/sbr_tf_grid.o $(intlib_faad)/specrec.o $(intlib_faad)/ssr.o $(intlib_faad)/ssr_fb.o $(intlib_faad)/ssr_ipqf.o $(intlib_faad)/syntax.o $(intlib_faad)/tns.o
intlib_split=libmp4splitter
OBJS+= $(intlib_split)/Ap4MSHL2FileByteStream.o $(intlib_split)/mp4splitter.o
OBJS+= $(intlib_split)/Atoms/Ap4Atom.o $(intlib_split)/Atoms/Ap4AtomFactory.o $(intlib_split)/Atoms/Ap4AtomSampleTable.o $(intlib_split)/Atoms/Ap4ContainerAtom.o $(intlib_split)/Atoms/Ap4CttsAtom.o $(intlib_split)/Atoms/Ap4DrefAtom.o $(intlib_split)/Atoms/Ap4EsdsAtom.o $(intlib_split)/Atoms/Ap4FtypAtom.o $(intlib_split)/Atoms/Ap4HdlrAtom.o $(intlib_split)/Atoms/Ap4HmhdAtom.o $(intlib_split)/Atoms/Ap4MdhdAtom.o $(intlib_split)/Atoms/Ap4MoovAtom.o $(intlib_split)/Atoms/Ap4MvhdAtom.o $(intlib_split)/Atoms/Ap4NmhdAtom.o $(intlib_split)/Atoms/Ap4SdpAtom.o $(intlib_split)/Atoms/Ap4SmhdAtom.o $(intlib_split)/Atoms/Ap4StcoAtom.o $(intlib_split)/Atoms/Ap4StscAtom.o $(intlib_split)/Atoms/Ap4StsdAtom.o $(intlib_split)/Atoms/Ap4StssAtom.o $(intlib_split)/Atoms/Ap4StszAtom.o $(intlib_split)/Atoms/Ap4SttsAtom.o $(intlib_split)/Atoms/Ap4TkhdAtom.o $(intlib_split)/Atoms/Ap4TrakAtom.o $(intlib_split)/Atoms/Ap4UrlAtom.o $(intlib_split)/Atoms/Ap4VmhdAtom.o
OBJS+= $(intlib_split)/Core/Ap4ByteStream.o $(intlib_split)/Core/Ap4DataBuffer.o $(intlib_split)/Core/Ap4DecoderConfigDescriptor.o $(intlib_split)/Core/Ap4DecoderSpecificInfoDescriptor.o $(intlib_split)/Core/Ap4Descriptor.o $(intlib_split)/Core/Ap4DescriptorFactory.o $(intlib_split)/Core/Ap4EsDescriptor.o $(intlib_split)/Core/Ap4File.o $(intlib_split)/Core/Ap4Movie.o $(intlib_split)/Core/Ap4Sample.o $(intlib_split)/Core/Ap4SampleDescription.o $(intlib_split)/Core/Ap4SampleEntry.o $(intlib_split)/Core/Ap4SampleTable.o $(intlib_split)/Core/Ap4SLConfigDescriptor.o $(intlib_split)/Core/Ap4Track.o $(intlib_split)/Core/Ap4UnknownDescriptor.o $(intlib_split)/Core/Ap4Utils.o

#INCDIR+=-I$(intlib_faad)
INCDIR+=-I$(intlib_split)/Atoms -I$(intlib_split)/Core

include ../makefile_after.mak

