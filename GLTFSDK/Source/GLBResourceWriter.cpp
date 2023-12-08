// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/GLBResourceWriter.h>

#include <sstream>

using namespace Microsoft::glTF;

namespace
{
    uint32_t CalculatePadding(size_t byteLength)
    {
        const uint32_t alignmentSize = GLB_CHUNK_ALIGNMENT_SIZE;

        const auto padAlign = byteLength % alignmentSize;
        const auto pad = padAlign ? alignmentSize - padAlign : 0U;

        return static_cast<uint32_t>(pad);
    }
}

GLBResourceWriter::GLBResourceWriter(std::shared_ptr<const IStreamWriter> streamWriter)

           : GLBResourceWriter(std::move(streamWriter), std::make_unique<std::stringstream>())
{

                     std::cout << "\n" << "2.2 GLBResourceWriter" << "\n";

                     std::cout << "2.2.1: m_stream: " << m_stream->tellp() << "\n";

}

GLBResourceWriter::GLBResourceWriter(std::shared_ptr<const IStreamWriter> streamWriter, std::unique_ptr<std::iostream> tempBufferStream) :

            GLTFResourceWriter(std::move(streamWriter)),

            m_stream(std::move(tempBufferStream))

            
{
                    std::cout << "\n" << "2.1. GLBResourceWriter Temp" << "\n";

                    std::cout << "2.1.1. m_stream: " << m_stream->tellp() << "\n";
                     

}

GLBResourceWriter::GLBResourceWriter(std::unique_ptr<IStreamWriterCache> streamCache)

             : GLBResourceWriter(std::move(streamCache), std::make_unique<std::stringstream>())
{

                     std::cout << "\n" << "III. GLBResourceWriter Cache" << "\n";

}

GLBResourceWriter::GLBResourceWriter(std::unique_ptr<IStreamWriterCache> streamCache, std::unique_ptr<std::iostream> tempBufferStream)

            : GLTFResourceWriter(std::move(streamCache)), m_stream(std::move(tempBufferStream))
{

                     std::cout << "\n" << "IV. GLBResourceWriter Cache Temp" << "\n";

}

void GLBResourceWriter::Flush(const std::string& manifest, const std::string& uri)
{
    uint32_t jsonChunkLength = static_cast<uint32_t>(manifest.length());
    const uint32_t jsonPaddingLength = ::CalculatePadding(jsonChunkLength);

    jsonChunkLength += jsonPaddingLength;

    uint32_t binaryChunkLength = static_cast<uint32_t>(GetBufferOffset(GLB_BUFFER_ID));
    const uint32_t binaryPaddingLength = ::CalculatePadding(binaryChunkLength);

    binaryChunkLength += binaryPaddingLength;

    const uint32_t length = GLB_HEADER_BYTE_SIZE // 12 bytes (GLB header) + 8 bytes (JSON header)
        + jsonChunkLength
        + sizeof(binaryChunkLength) + GLB_CHUNK_TYPE_SIZE // 8 bytes (BIN header)
        + binaryChunkLength;

    // The file stream
    std::cout << "\nURI : " << uri << "\n";
    std::shared_ptr<std::ostream> filestream = m_streamWriterCache->Get(uri);

    // Write GLB header (12 bytes)
    std::cout << "\nWrite GLB header (12 bytes) to File" << "\n";
    StreamUtils::WriteBinary(*filestream, GLB_HEADER_MAGIC_STRING, GLB_HEADER_MAGIC_STRING_SIZE);
    StreamUtils::WriteBinary(*filestream, GLB_HEADER_VERSION_2);
    StreamUtils::WriteBinary(*filestream, length);

    // Write JSON header (8 bytes)
    std::cout << "\nWrite JSON header (8 bytes) to File" << "\n";
    StreamUtils::WriteBinary(*filestream, jsonChunkLength);
    StreamUtils::WriteBinary(*filestream, GLB_CHUNK_TYPE_JSON, GLB_CHUNK_TYPE_SIZE);

    // Write JSON (indeterminate length)
    std::cout << "\nWrite JSON (indeterminate length) to File" << "\n";
    StreamUtils::WriteBinary(*filestream, manifest);


    if (jsonPaddingLength > 0)
    {
        std::cout << "\nWrite Padding to File" << "\n";
        // GLB spec requires the JSON chunk to be padded with trailing space characters (0x20) to satisfy alignment requirements
        StreamUtils::WriteBinary(*filestream, std::string(jsonPaddingLength, ' '));
    }

    // Write BIN header (8 bytes)
    std::cout << "\nWrite BIN header (8 bytes) to File" << "\n";
    StreamUtils::WriteBinary(*filestream, binaryChunkLength);
    StreamUtils::WriteBinary(*filestream, GLB_CHUNK_TYPE_BIN, GLB_CHUNK_TYPE_SIZE);

    // Write BIN contents (indeterminate length) - copy the temporary buffer's contents to the output stream
    std::cout << "\nWrite BIN contents to File";
    *filestream << m_stream->rdbuf();

    if (binaryPaddingLength > 0)
    {
        std::cout << "\nWrite BIN padding to File";
        // GLB spec requires the BIN chunk to be padded with trailing zeros (0x00) to satisfy alignment requirements
        StreamUtils::WriteBinary(*filestream, std::vector<uint8_t>(binaryPaddingLength, 0));
    }
}

std::string GLBResourceWriter::GenerateBufferUri(const std::string& bufferId) const
{
    std::cout << "\n" << "5. GenerateBufferUri" << "\n";

    std::string bufferUri;

    // Return an empty uri string when passed the GLB buffer id
    if (bufferId != GLB_BUFFER_ID)
    {
        bufferUri = GLTFResourceWriter::GenerateBufferUri(bufferId);
    }

    return bufferUri;
}

std::ostream* GLBResourceWriter::GetBufferStream(const std::string& bufferId)
{
    std::cout << "\n" << "8. Get Stream" << "\n";

    std::ostream* stream = m_stream.get();
    
    if (bufferId != GLB_BUFFER_ID)
    {
        stream = GLTFResourceWriter::GetBufferStream(bufferId);
    }

    return stream;
}