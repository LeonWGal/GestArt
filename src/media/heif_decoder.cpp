#include "heif_decoder.h"
#include <QFile>
#include <QColorSpace>
#include <memory>
#include <mutex>
#include <libheif/heif.h>

QSize heifNaturalSize(const QString& path){
    QFile file(path);if(!file.open(QIODevice::ReadOnly)||file.size()>256ll*1024*1024)return {};
    const auto bytes=file.readAll();
    std::unique_ptr<heif_context,decltype(&heif_context_free)> context(heif_context_alloc(),heif_context_free);
    if(!context || heif_context_read_from_memory_without_copy(context.get(),bytes.data(),size_t(bytes.size()),nullptr).code!=heif_error_Ok)return {};
    heif_image_handle* raw=nullptr;
    if(heif_context_get_primary_image_handle(context.get(),&raw).code!=heif_error_Ok)return {};
    std::unique_ptr<heif_image_handle,decltype(&heif_image_handle_release)> handle(raw,heif_image_handle_release);
    return {heif_image_handle_get_width(handle.get()),heif_image_handle_get_height(handle.get())};
}

QImage decodeHeif(const QString& path,QSize requested,QString* error){
    // Bound concurrent native decoder working buffers in addition to the byte cache.
    static std::mutex decodeMutex;
    std::lock_guard lock(decodeMutex);
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)){if(error)*error=file.errorString();return {};}
    if(file.size()>256ll*1024*1024){if(error)*error="HEIF file exceeds the input budget";return {};}
    const QByteArray bytes=file.readAll();
    std::unique_ptr<heif_context,decltype(&heif_context_free)> context(heif_context_alloc(),heif_context_free);
    if(!context){if(error)*error="Cannot allocate HEIF context";return {};}
    heif_context_set_max_decoding_threads(context.get(),2);
    auto result=heif_context_read_from_memory_without_copy(context.get(),bytes.data(),size_t(bytes.size()),nullptr);
    if(result.code!=heif_error_Ok){if(error)*error=QString::fromUtf8(result.message);return {};}
    heif_image_handle* rawHandle=nullptr;
    result=heif_context_get_primary_image_handle(context.get(),&rawHandle);
    if(result.code!=heif_error_Ok){if(error)*error=QString::fromUtf8(result.message);return {};}
    std::unique_ptr<heif_image_handle,decltype(&heif_image_handle_release)> handle(rawHandle,heif_image_handle_release);
    const int width=heif_image_handle_get_width(handle.get()),height=heif_image_handle_get_height(handle.get());
    if(width<=0 || height<=0 || qint64(width)*height*4>256ll*1024*1024){if(error)*error="HEIF image exceeds the decoded pixel budget";return {};}
    heif_image* rawImage=nullptr;
    result=heif_decode_image(handle.get(),&rawImage,heif_colorspace_RGB,heif_chroma_interleaved_RGBA,nullptr);
    if(result.code!=heif_error_Ok){if(error)*error=QString::fromUtf8(result.message);return {};}
    std::unique_ptr<heif_image,decltype(&heif_image_release)> decoded(rawImage,heif_image_release);
    int stride=0;const auto* pixels=heif_image_get_plane_readonly(decoded.get(),heif_channel_interleaved,&stride);
    if(!pixels){if(error)*error="HEIF decoder returned no pixels";return {};}
    QImage image=QImage(pixels,heif_image_get_width(decoded.get(),heif_channel_interleaved),heif_image_get_height(decoded.get(),heif_channel_interleaved),stride,QImage::Format_RGBA8888).copy();
    const size_t profileSize=heif_image_handle_get_raw_color_profile_size(handle.get());
    if(profileSize>0 && profileSize<4*1024*1024){
        QByteArray profile(qsizetype(profileSize),Qt::Uninitialized);
        if(heif_image_handle_get_raw_color_profile(handle.get(),profile.data()).code==heif_error_Ok)image.setColorSpace(QColorSpace::fromIccProfile(profile));
    }
    if(image.colorSpace().isValid())image.convertToColorSpace(QColorSpace::SRgb);else image.setColorSpace(QColorSpace::SRgb);
    if(requested.isValid() && (image.width()>requested.width() || image.height()>requested.height()))image=image.scaled(requested,Qt::KeepAspectRatio,Qt::SmoothTransformation);
    return image;
}
