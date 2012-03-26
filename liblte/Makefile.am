# Copyright 2012 Ben Wojtowicz
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Affero General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Affero General Public License for more details.
#
#    You should have received a copy of the GNU Affero General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

include $(top_srcdir)/Makefile.common

INCLUDES = \
	-Ihdr/ \
	-I$(top_srcdir)/cmn_hdr/

noinst_LTLIBRARIES = \
	lib/liblte.la

lib_liblte_la_SOURCES = \
	src/liblte_phy.cc \
	src/liblte_rrc.cc
