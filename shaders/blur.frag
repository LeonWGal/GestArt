#version 440
layout(location=0) in vec2 qt_TexCoord0;
layout(location=0) out vec4 fragColor;
layout(std140,binding=0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    vec2 sampleStep;
    float kernelRadius;
} ubuf;
layout(binding=1) uniform sampler2D source;
void main(){
    vec2 uv=qt_TexCoord0;
    vec4 color=texture(source,uv);
    float total=1.0;
    float sigma=max(ubuf.kernelRadius/3.0,.001);
    // Pair adjacent physical pixels with bilinear sampling; no holes in the kernel.
    for(int i=1;i<=64;i+=2){
        if(float(i)>ubuf.kernelRadius)break;
        float a=exp(-.5*float(i*i)/(sigma*sigma));
        float b=float(i+1)<=ubuf.kernelRadius?exp(-.5*float((i+1)*(i+1))/(sigma*sigma)):0.0;
        float weight=a+b;
        vec2 offset=ubuf.sampleStep*(float(i)+b/weight);
        color+=(texture(source,uv+offset)+texture(source,uv-offset))*weight;
        total+=2.0*weight;
    }
    fragColor=color/total*ubuf.qt_Opacity;
}
