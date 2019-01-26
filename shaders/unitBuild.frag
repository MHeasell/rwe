#version 130

in vec2 fragTexCoord;
in float height;
in vec3 worldNormal;
out vec4 outColor;

uniform sampler2D textureSampler;
uniform float unitY;
uniform float seaLevel;
uniform bool shade;
uniform float percentComplete;
uniform float time;

const vec3 waterTint = vec3(0.5, 0.5, 1.0);
const vec3 normalTint = vec3(1.0, 1.0, 1.0);
const vec3 lightDirection = normalize(vec3(-1.0, 4.0, 1.0));

const float band1EndPercent = 0.05;
const float band1Speed = 900.0;
const float band1Thickness = 4.0;

const float band2EndPercent = 0.15;
const float band2Speed = 900.0;
const float band2Thickness = 4.0;

const float mainFillStartPercent = 0.2;
const float mainFillSpeed = 900.0;
const float mainFillLeadThickness = 4.0;

const float textureFillStartPercent = 0.7;
const float textureFillSpeed = 900.0;
const float textureFillLeadThickness = 4.0;

const float band3EndPercent = 0.95;
const float band3Speed = 900.0;
const float band3Thickness = 4.0;

vec3 shadeNormal()
{
    vec3 baseColor = vec3(texture(textureSampler, fragTexCoord));
    float lightIntensity = shade
        ? 1.5 * clamp(dot(worldNormal, lightDirection), 0.0, 1.0) + 0.5
        : 1.0;
    return baseColor * lightIntensity * (height > seaLevel ? normalTint : waterTint);
}

void main(void)
{
    float posY = height - unitY;

    //0 = transparent, 1 = sine green, 2 = cosine green, 3 = ignore (texture pass fills this in)
    int shadingMethod = 0;
    //This is where we determine what shading method to use on this pixel
    //Order of overlays here is in highest to lowest priority

    //third band pass
    if (1 - (percentComplete - band3EndPercent)*band3Speed >= posY &&
        posY + band3Thickness >= (1 - (percentComplete - band3EndPercent)*band3Speed))
            shadingMethod = 2;

    //second band pass
    else if (1 - (percentComplete - band2EndPercent)*band2Speed >= posY &&
        posY + band2Thickness >= (1 - (percentComplete - band2EndPercent)*band2Speed))
            shadingMethod = 2;

    //first band pass
    else if (1 - (percentComplete - band1EndPercent)*band1Speed >= posY &&
        posY + band1Thickness >= (1 - (percentComplete - band1EndPercent)*band1Speed))
            shadingMethod = 2;

    //texture overlay
    else if ((percentComplete - textureFillStartPercent)*textureFillSpeed >= posY) {
        //lead band
        if (posY + textureFillLeadThickness >= (percentComplete - textureFillStartPercent)*textureFillSpeed)
            shadingMethod = 2;
        //rest of fill
        else
            shadingMethod = 3;
    }

    //main color overlay
    else if ((percentComplete - mainFillStartPercent)*mainFillSpeed >= posY) {
        //lead band
        if (posY + mainFillLeadThickness >= (percentComplete - mainFillStartPercent)*mainFillSpeed)
            shadingMethod = 2;
        //rest of fill
        else
            shadingMethod = 1;
    }

    //Now we actually compute pixel color
    vec4 color = vec4(0,0,0,1);

    //Shading methods very simple right now, however can be expanded upon
    //for more complex effects at high fidelity.
    //I tried to emulate the TA color movement but my attempts looked bad.
    //Just pure green ended up looking better.

    //0 = transparent, 1 = sine green, 2 = cosine green, 3 = ignore (texture pass fills this in)

    float time2 = time / 15.0;

    if (shadingMethod == 3) {
        color = vec4(shadeNormal(), 1.0);
    }
    else if (shadingMethod == 0)
        color.a = 0;
    else if (shadingMethod == 1) {
        color.g = (0.5*sin(time2))+0.5;
    }
    else if (shadingMethod == 2) {
        color.g = (0.5*cos(time2))+0.5;
    }

    outColor = color;
}
