/****************************************************************************
*
*                            Open Watcom Project
*
*  Copyright (c) 2004-2008 The Open Watcom Contributors. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  WGML tags :AUTHOR processing
*
****************************************************************************/


#include    "wgml.h"


/***************************************************************************/
/*  :author tag                                                             */
/***************************************************************************/

void gml_author( const gmltag * entry )
{
    char        *   p;
    font_number     font_save;
    page_pos        old_line_pos;
    uint32_t        left_indent;
    uint32_t        right_indent;

    if( !((ProcFlags.doc_sect == doc_sect_titlep) ||
          (ProcFlags.doc_sect_nxt == doc_sect_titlep)) ) {
        xx_nest_err_cc( err_tag_wrong_sect, entry->tagname, ":TITLEP section" );
    }
    p = scan_start;
    if( *p && *p == '.' ) p++;          // over . to docnum

    while( *p == ' ' ) {                // over WS to <text line>
        p++;
    }

    /* Only first AUTHOR is used in banners */

    if( GlobalFlags.firstpass && !ProcFlags.author_tag_seen ) {
        if( *p ) {
            add_symvar( &global_dict, "$author", p, no_subscript, 0 );
        }
        ProcFlags.author_tag_seen = true;
    }

    scr_process_break();
    start_doc_sect();                       // if not already done

    font_save = g_curr_font;
    g_curr_font = layout_work.author.font;
    g_text_spacing = layout_work.titlep.spacing;

    /************************************************************/
    /*  pre_skip and skip are treated as pre_top_skip because   */
    /*  they are always used at the top of the page             */
    /*  this is not what the docs say, at least about pre_skip  */
    /************************************************************/

    if( !ProcFlags.author_tag_seen ) {
        set_skip_vars( NULL, &layout_work.author.pre_skip, NULL, g_text_spacing, g_curr_font );
    } else {
        set_skip_vars( NULL, &layout_work.author.skip, NULL, g_text_spacing, g_curr_font );
    }

    left_indent = conv_hor_unit( &layout_work.author.left_adjust, g_curr_font );
    right_indent = conv_hor_unit( &layout_work.author.right_adjust, g_curr_font );

    t_page.cur_left += left_indent;
    t_page.cur_width = t_page.cur_left;
    if( t_page.max_width < right_indent ) {
        t_page.max_width = 0;               // negative right margin not allowed
        xx_line_err( err_page_width_too_small, val_start );
    } else {
        t_page.max_width -= right_indent;
    }
    ProcFlags.keep_left_margin = true;  // keep special indent
    old_line_pos = line_position;
    line_position = layout_work.author.page_position;
    ProcFlags.as_text_line = true;
    if( *p ) {
        process_text( p, g_curr_font );
    } else {
        ProcFlags.titlep_starting = true;
    }
    scr_process_break();                // commit author line (or blank line)

    line_position = old_line_pos;
    g_curr_font = font_save;
    scan_start = scan_stop + 1;
}
