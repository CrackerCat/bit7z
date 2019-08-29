// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2018  Riccardo Ostani - All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * Bit7z is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bit7z; if not, see https://www.gnu.org/licenses/.
 */

#include "../include/bitstreamextractor.hpp"

#include "7zip/Archive/IArchive.h"

#include "../include/bitexception.hpp"
#include "../include/extractcallback.hpp"
#include "../include/memextractcallback.hpp"
#include "../include/streamextractcallback.hpp"

using namespace bit7z;

BitStreamExtractor::BitStreamExtractor( const Bit7zLibrary& lib, const BitInFormat& format )
    : BitArchiveOpener( lib, format ) {
    if ( format == BitFormat::Auto ) {
        throw BitException( "Automatic format detection not supported for in-memory archives" );
    }
}

void BitStreamExtractor::extract( istream& in_stream, const wstring& out_dir ) const {
    BitInputArchive in_archive( *this, in_stream );

    auto* extract_callback_spec = new ExtractCallback( *this, in_archive, L"", out_dir );
    CMyComPtr< IArchiveExtractCallback > extract_callback( extract_callback_spec );
    HRESULT res = in_archive.extract( vector< uint32_t >(), extract_callback );
    if ( res != S_OK ) {
        throw BitException( extract_callback_spec->getErrorMessage() );
    }
}

void BitStreamExtractor::extract( istream& in_stream, vector< byte_t >& out_buffer, unsigned int index ) const {
    BitInputArchive in_archive( *this, in_stream );

    uint32_t number_items = in_archive.itemsCount();
    if ( index >= number_items ) {
        throw BitException( L"Index " + std::to_wstring( index ) + L" is out of range" );
    }

    if ( in_archive.isItemFolder( index ) ) { //Consider only files, not folders
        throw BitException( "Cannot extract a folder to a buffer" );
    }

    map< wstring, vector< byte_t > > buffers_map;
    auto* extract_callback_spec = new MemExtractCallback( *this, in_archive, buffers_map );
    CMyComPtr< IArchiveExtractCallback > extract_callback( extract_callback_spec );

    const vector< uint32_t > indices( 1, index );
    HRESULT res = in_archive.extract( indices, extract_callback );
    if ( res != S_OK ) {
        throw BitException( extract_callback_spec->getErrorMessage() );
    }
    out_buffer = std::move( buffers_map.begin()->second );
}

void BitStreamExtractor::extract( istream& in_stream, std::ostream& out_stream, unsigned int index ) const {
    BitInputArchive in_archive( *this, in_stream );

    uint32_t number_items = in_archive.itemsCount();
    if ( index >= number_items ) {
        throw BitException( L"Index " + std::to_wstring( index ) + L" is out of range" );
    }

    if ( in_archive.isItemFolder( index ) ) { //Consider only files, not folders
        throw BitException( "Cannot extract a folder to a buffer" );
    }

    auto* extract_callback_spec = new StreamExtractCallback( *this, in_archive, out_stream );
    CMyComPtr< IArchiveExtractCallback > extract_callback( extract_callback_spec );

    const vector< uint32_t > indices( 1, index );
    HRESULT res = in_archive.extract( indices, extract_callback );
    if ( res != S_OK ) {
        throw BitException( extract_callback_spec->getErrorMessage() );
    }
}

void BitStreamExtractor::extract( istream& in_stream, map< wstring, vector< byte_t > >& out_map ) const {
    BitInputArchive in_archive( *this, in_stream );

    uint32_t number_items = in_archive.itemsCount();
    vector< uint32_t > files_indices;
    for ( uint32_t i = 0; i < number_items; ++i ) {
        if ( !in_archive.isItemFolder( i ) ) { //Consider only files, not folders
            files_indices.push_back( i );
        }
    }

    auto* extract_callback_spec = new MemExtractCallback( *this, in_archive, out_map );
    CMyComPtr< IArchiveExtractCallback > extract_callback( extract_callback_spec );
    HRESULT res = in_archive.extract( files_indices, extract_callback );
    if ( res != S_OK ) {
        throw BitException( extract_callback_spec->getErrorMessage() );
    }
}

void BitStreamExtractor::test( istream& in_stream ) const {
    BitInputArchive in_archive( *this, in_stream );

    map< wstring, vector< byte_t > > dummy_map; //output map (not used since we are testing!)
    auto* extract_callback_spec = new MemExtractCallback( *this, in_archive, dummy_map );
    CMyComPtr< IArchiveExtractCallback > extract_callback( extract_callback_spec );

    HRESULT res = in_archive.test( extract_callback );
    if ( res != S_OK ) {
        throw BitException( extract_callback_spec->getErrorMessage() );
    }
}