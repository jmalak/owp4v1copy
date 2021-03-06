/****************************************************************************
*
*                            Open Watcom Project
*
*  Copyright (c) 2004-2010 The Open Watcom Contributors. All Rights Reserved.
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
* Description:  WGML tags :HDREF :FIGREF :FNREF processing
*
****************************************************************************/


#include    "wgml.h"


static  bool    ref_page        = false;
static  bool    page_found      = false;
static  bool    refid_found     = false;
static  char    refid[ID_LEN];

/***************************************************************************/
/* Get attribute values for FIGREF, FNREF, and HDREF                       */
/***************************************************************************/

static char * get_ref_attributes( void )
{
    char        *   p;
    char        *   pa;

    scan_err = false;
    p = scan_start;

    if( *p == '.' ) {
        /* already at tag end */
    } else {
        for( ;; ) {
            pa = get_att_start( p );
            p = att_start;
            if( ProcFlags.reprocess_line ) {
                break;
            }
            if( !strnicmp( "page", p, 4 ) ) {
                page_found = true;
                p += 4;
                p = get_att_value( p );
                if( val_start == NULL ) {
                    break;
                }
                if( !strnicmp( "yes", val_start, 3 ) ) {
                    ref_page = true;
                } else if( !strnicmp( "no", val_start, 2 ) ) {
                    ref_page = false;
                } else {
                    xx_line_err( err_inv_att_val, val_start );
                    scan_start = scan_stop + 1;
                    return( p );
                }
                if( ProcFlags.tag_end_found ) {
                    break;
                }
            } else if( !strnicmp( "refid", p, 5 ) ) {
                p += 5;
                p = get_refid_value( p, refid );
                if( val_start == NULL ) {
                    break;
                }
                refid_found = true;
                if( ProcFlags.tag_end_found ) {
                    break;
                }
            } else {    // no match = end-of-tag in wgml 4.0
                p = pa; // restore any spaces before non-attribute value
                break;
            }
        }
    }
    if( !refid_found ) {            // detect missing required attribute
        xx_err( err_att_missing );
        scan_start = scan_stop + 1;
    }
    return( p );
}

/***************************************************************************/
/*      :FIGREF refid=�id-name�                                            */
/*              [page=yes                                                  */
/*                    no].                                                 */
/* This tag causes a figure reference to be generated. The text "Figure"   */
/* followed by the figure number will be generated at the point where the  */
/* :figref tag is specified. The figure reference tag is a paragraph       */
/* element, and is used with text to create the content of a basic         */
/* document element. The figure being referenced must have a figure        */
/* caption specified.                                                      */
/***************************************************************************/

void gml_figref( const gmltag * entry )
{
    bool            do_page     =   false;  // default for fwd refs w/no "page" attribute
    char            buffer[11];
    char        *   p;
    char        *   ref_text;
    ref_entry   *   cur_re;
    size_t          bu_len;
    size_t          len;

    static  char    def_page[]  = " on page XXX";
    static  char    def_ref[]   = "Figure XX";
    static  char    on_page[]  = " on page ";
    static  size_t  dp_len;
    static  size_t  dr_len;
    static  size_t  op_len;

    p = get_ref_attributes();

    cur_re = find_refid( fig_ref_dict, refid );

    if( page_found ) {
        do_page = ref_page;
        page_found = false;
        ref_page = false;
    } else if( cur_re != NULL ) {
        do_page = ((page + 1) != cur_re->u.ffh.entry->pageno);
    }

    dp_len = strlen( def_page );
    dr_len = strlen( def_ref );
    op_len = strlen( on_page );
    if( cur_re == NULL ) {              // undefined refid
        if( do_page ) {
            ref_text = (char *) mem_alloc( dr_len + dp_len + 1 );
            strcpy( ref_text, def_ref );
            strcat( ref_text, def_page );
        } else {
            ref_text = (char *) mem_alloc( dr_len + 1 );
            strcpy( ref_text, def_ref );
        }
    } else {
        len = strlen( cur_re->u.ffh.entry->prefix );
        if( do_page ) {
            format_num( cur_re->u.ffh.entry->pageno, buffer, sizeof( buffer ),
                        cur_re->u.ffh.entry->style );
            bu_len = strlen( buffer );
            ref_text = (char *) mem_alloc( len + op_len + bu_len  + 1 );
            strcpy_s( ref_text, len + 1, cur_re->u.ffh.entry->prefix );
            ref_text[len - 1] = '\0';       // remove delim
            len += (dr_len + op_len + bu_len );
            strcat( ref_text, on_page );
            strcat_s( ref_text, len + 1, buffer );
        } else {
            ref_text = (char *) mem_alloc( len + 1 );
            strcpy_s( ref_text, len + 1, cur_re->u.ffh.entry->prefix );
            ref_text[len - 1] = '\0';       // remove delim
        }
    }
    process_text( ref_text, g_curr_font );
    mem_free( ref_text );
    ref_text = NULL;

    if( !ProcFlags.reprocess_line && *p ) {
        if( *p == '.' ) p++;                // possible tag end
        if( *p ) {                          // only if text follows
            post_space = 0;                 // cancel space after ref_text
            process_text( p, g_curr_font );
        }
    }

    if( GlobalFlags.lastpass ) {
        if( cur_re == NULL ) {
            if( passes == 1 ) {
                fig_fwd_refs = init_fwd_ref( fig_fwd_refs, refid );
            } else {
                undef_id_warn_info( refid, "figure" );
            }
        }
    }

    scan_start = scan_stop + 1;

    return;
}

/***************************************************************************/
/*         :HDREF refid='id-name'                                          */
/*                [page=yes                                                */
/*                      no].                                               */
/*                                                                         */
/* This tag causes a heading reference to be generated.  The heading       */
/* reference tag is a paragraph element, and is used with text to create   */
/* the content of a basic document element.  The heading text from the     */
/* referenced heading is enclosed in double quotation marks and inserted   */
/* into the formatted document.                                            */
/***************************************************************************/

void gml_hdref( const gmltag * entry )
{
    bool            do_page     =   false;  // default for fwd refs w/no "page" attribute
    char            buffer[11];
    char        *   p;
    char        *   ref_text;
    ref_entry   *   cur_re;
    size_t          bu_len;
    size_t          len;

    static  char    def_page[]  = " on page XXX";
    static  char    def_ref[]   = "\"Undefined Heading\"";
    static  char    on_page[]  = " on page ";
    static  size_t  dp_len;
    static  size_t  dr_len;
    static  size_t  op_len;

    p = get_ref_attributes();

    cur_re = find_refid( hd_ref_dict, refid );

    if( page_found ) {
        do_page = ref_page;
        page_found = false;
        ref_page = false;
    } else if( cur_re != NULL ) {
        do_page = ((page + 1) != cur_re->u.ffh.entry->pageno);
    }

    dp_len = strlen( def_page );
    dr_len = strlen( def_ref );
    op_len = strlen( on_page );
    if( cur_re == NULL ) {              // undefined refid
        if( do_page ) {
            ref_text = (char *) mem_alloc( dr_len + dp_len + 1 );
            strcpy( ref_text, def_ref );
            strcat( ref_text, def_page );
        } else {
            ref_text = (char *) mem_alloc( dr_len + 1 );
            strcpy( ref_text, def_ref );
        }
    } else {
        len = strlen( cur_re->u.ffh.entry->text ) + 2;        // allow for quote chars
        if( do_page ) {
            format_num( cur_re->u.ffh.entry->pageno, buffer, sizeof( buffer ),
                        cur_re->u.ffh.entry->style );
            bu_len = strlen( buffer );
            len += (op_len + bu_len );
            ref_text = (char *) mem_alloc( len + 1 );
            strcpy( ref_text, "\"" );
            strcat_s( ref_text, len + 1, cur_re->u.ffh.entry->text );
            strcat( ref_text, "\"" );
            strcat( ref_text, on_page );
            strcat_s( ref_text, len + 1, buffer );
        } else {
            ref_text = (char *) mem_alloc( len + 1 );
            strcpy( ref_text, "\"" );
            strcat_s( ref_text, len + 1, cur_re->u.ffh.entry->text );
            strcat( ref_text, "\"" );
        }
    }
    process_text( ref_text, g_curr_font );
    mem_free( ref_text );
    ref_text = NULL;

    if( !ProcFlags.reprocess_line && *p ) {
        if( *p == '.' ) p++;                // possible tag end
        if( *p ) {                          // only if text follows
            post_space = 0;                 // cancel space after ref_text
            process_text( p, g_curr_font );
        }
    }

    if( GlobalFlags.lastpass ) {
        if( cur_re == NULL ) {
            if( passes == 1 ) {
                hd_fwd_refs = init_fwd_ref( hd_fwd_refs, refid );
            } else {
                undef_id_warn_info( refid, "heading" );
            }
        }
    }

    scan_start = scan_stop + 1;

    return;
}

/***************************************************************************/
/*      :FNREF refid=�id-name�                                             */
/* This tag causes a footnote reference to be generated. The number of the */
/* referenced footnote will be generated at the point where the :fnref tag */
/* is specified. The footnote reference tag is a paragraph element, and is */
/* used with text to create the content of a basic document element.       */
/***************************************************************************/

void gml_fnref( const gmltag * entry )
{
    char            buffer[11];
    char        *   p;
    ref_entry   *   cur_re;

    p = get_ref_attributes();

    /***********************************************************************/
    /* wgml 4.0: FNREF changes the font to "0" and does not change it back */
    /* reactivate the next line and change the font used with the first    */
    /* process_text() to g_curr_font to restore this behavior              */
    /* our wgml does something a bit more sensible                         */
    /***********************************************************************/

//    g_curr_font = FONT0;                // layout attribute "font" is ignored

    cur_re = find_refid( fn_ref_dict, refid );
    if( cur_re == NULL ) {              // undefined refid
        process_text( "(XX)", g_curr_font );
    } else {
        format_num( cur_re->u.ffh.entry->number, buffer, sizeof( buffer ),
                    layout_work.fnref.number_style );
        input_cbs->fmflags &= ~II_eol;
        process_text( buffer, layout_work.fnref.font );
    }

    if( !ProcFlags.reprocess_line && *p ) {
        if( *p == '.' ) p++;                // possible tag end
        if( *p ) {                          // only if text follows
            post_space = 0;                 // cancel space after ref_text
            process_text( p, g_curr_font );
        }
    }

    if( GlobalFlags.lastpass ) {
        if( cur_re == NULL ) {
            if( passes == 1 ) {
                fn_fwd_refs = init_fwd_ref( fn_fwd_refs, refid );
            } else {
                undef_id_warn_info( refid, "footnote" );
            }
        }
    }

    scan_start = scan_stop + 1;
    return;
}


