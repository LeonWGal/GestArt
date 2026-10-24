#version 440
layout(location=0) in vec2 qt_TexCoord0;
layout(location=0) out vec4 fragColor;
layout(std140,binding=0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    vec2 cropOffset;
    vec2 panelSize;
    vec2 cropSize;
    vec2 sampleStep;
    float kernelRadius;
    float cornerRadius;
    vec4 tint;
} ubuf;
layout(binding=1) uniform sampler2D source;
void main(){
    vec2 uv=(ubuf.cropOffset+qt_TexCoord0*ubuf.panelSize)/ubuf.cropSize;
    vec3 rgb=texture(source,uv).rgb;
    float total=1.0;
    float sigma=max(ubuf.kernelRadius/3.0,.001);
    for(int i=1;i<=64;i+=2){
        if(float(i)>ubuf.kernelRadius)break;
        float a=exp(-.5*float(i*i)/(sigma*sigma));
        float b=float(i+1)<=ubuf.kernelRadius?exp(-.5*float((i+1)*(i+1))/(sigma*sigma)):0.0;
        float weight=a+b;
        vec2 offset=ubuf.sampleStep*(float(i)+b/weight);
        rgb+=(texture(source,uv+offset).rgb+texture(source,uv-offset).rgb)*weight;
        total+=2.0*weight;
    }
    rgb/=total;
    vec2 halfSize=ubuf.panelSize*.5;
    float radius=min(ubuf.cornerRadius,min(halfSize.x,halfSize.y));
    vec2 distance=abs(qt_TexCoord0*ubuf.panelSize-halfSize)-halfSize+radius;
    float sdf=length(max(distance,vec2(0)))+min(max(distance.x,distance.y),0.0)-radius;
    float coverage=1.0-smoothstep(-.5,.5,sdf);
    vec3 color=mix(rgb,ubuf.tint.rgb/max(.001,ubuf.tint.a),ubuf.tint.a);
    fragColor=vec4(color,1)*coverage*ubuf.qt_Opacity;
}
