/****************************************************************************
*
*                            Open Watcom Project
*
*  Copyright (c) 2004-2009 The Open Watcom Contributors. All Rights Reserved.
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
* Description:  Implementation of CPrintInfo.
*
****************************************************************************/


#include "stdafx.h"

CPrintInfo::CPrintInfo()
/**********************/
{
    m_pPD = new CPrintDialog( FALSE );
    m_pPD->m_pd.nMinPage = 1;
    m_pPD->m_pd.nMaxPage = 0xFFFF;
    m_bPreview = FALSE;
    m_bDirect = FALSE;
    m_bContinuePrinting = TRUE;
    m_nCurPage = 1;
    m_lpUserData = NULL;
}

CPrintInfo::~CPrintInfo()
/***********************/
{
    ASSERT( m_pPD != NULL );
    if( m_pPD->m_pd.hDC != NULL ) {
        ::DeleteDC( m_pPD->m_pd.hDC );
    }
    delete m_pPD;
}
