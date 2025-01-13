#version 450


layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec3 inNormal;


layout(location = 0) out vec4 outColour;


//enable uniformShaderInputLayout.ObjectInputs.useMaterialData in pipeline configuration
layout(set = 0, binding = 1) uniform materialUniformBufferObject {
	vec4 objColour;
	float diffuseConstant;
	float specularConstant;
	float shininess;
};

//enable uniformShaderInputLayout.ObjectInputs.useCamTransform in pipeline configuration
layout(set = 0, binding = 2) uniform CameraUniformBufferObject {
	mat4 camTransform;
};

//enable uniformShaderInputLayout.GlobalInputs.useDirectionalLight in pipeline configuration
layout(set = 1, binding = 0) uniform LightUniformBufferObject {
	vec4 lightColour;
	vec3 lightDirection;
	float lightIntensity;
	float ambientLightIntensity;
};


vec4 rgb_to_hsv(vec4 rgb) { 
  
    // R, G, B values are divided by 255 
    // to change the range from 0..255 to 0..1 
	

    // h, s, v = hue, saturation, value 
    float cmax = max(rgb.r, max(rgb.g, rgb.b)); // maximum of r, g, b 
    float cmin = min(rgb.r, min(rgb.g, rgb.b)); // minimum of r, g, b 
    float diff = cmax - cmin; // diff of cmax and cmin. 
	
	vec4 hsv = rgb;
    // if cmax and cmax are equal then h = 0 
    if (cmax == cmin) {
        hsv[0] = 0; 
	}
  
    // if cmax equal r then compute h 
    else if (cmax == rgb.r) 
        hsv[0] = mod(60 * ((rgb.g - rgb.b) / diff) + 360, 360); 
  
    // if cmax equal g then compute h 
    else if (cmax == rgb.g) 
        hsv[0] = mod(60 * ((rgb.b - rgb.r) / diff) + 120, 360); 
  
    // if cmax equal b then compute h 
    else if (cmax == rgb.b) 
        hsv[0] = mod(60 * ((rgb.r - rgb.g) / diff) + 240, 360); 
  
    // if cmax equal zero 
    if (cmax == 0) 
        hsv[1] = 0; 
    else
        hsv[1] = (diff / cmax); 
  
    // compute v 
    hsv[2] = cmax; 

	return hsv;
} 

vec4 hsv_to_rgb(vec4 hsv) {
    // Normalize H to [0, 360]
    while (hsv[0] < 0) hsv[0] += 360.0f;
    while (hsv[0] >= 360) hsv[0] -= 360.0f;

    vec4 rgb = hsv;

    if (hsv[1] <= 0.0f) {
        // If saturation is 0, color is grayscale
        rgb[0] = hsv[2];
        rgb[1] = hsv[2];
        rgb[2] = hsv[2];
    } else {
        float C = hsv[2] * hsv[1];            // Chroma
        float X = C * (1.0f - abs(mod(hsv[0] / 60.0f, 2) - 1.0f)); // Second largest component
        float m = hsv[2] - C;

        if (hsv[0] < 60) {
            rgb[0] = C; rgb[1] = X; rgb[2] = 0;
        } else if (hsv[0] < 120) {
            rgb[0] = X; rgb[1] = C; rgb[2] = 0;
        } else if (hsv[0] < 180) {
            rgb[0] = 0; rgb[1] = C; rgb[2] = X;
        } else if (hsv[0] < 240) {
            rgb[0] = 0; rgb[1] = X; rgb[2] = C;
        } else if (hsv[0] < 300) {
            rgb[0] = X; rgb[1] = 0; rgb[2] = C;
        } else {
            rgb[0] = C; rgb[1] = 0; rgb[2] = X;
        }

        // Add the adjustment factor (m) to bring the RGB values to the correct range
        rgb[0] += m;
        rgb[1] += m;
        rgb[2] += m;
    }

    return rgb;
}

void main() {
	vec3 to_light = -lightDirection;
	vec3 reflection = 2.0 * dot(inNormal,to_light) * inNormal - to_light;
	vec3 to_camera = camTransform[3].xyz - inPosition.xyz;

	reflection = normalize( reflection );
	to_camera = normalize( to_camera );

	float cos_angle = dot(reflection, to_camera);
	cos_angle = clamp(cos_angle, 0.0, 1.0);
	cos_angle = pow(cos_angle, shininess);

	outColour = vec4(lightColour.xyz*objColour.xyz,objColour.w); 
	
	float specular_scale = specularConstant*cos_angle;
	float diffuse_scale = diffuseConstant*clamp(dot(to_light, inNormal),0,1);

    float intensity = specular_scale + diffuse_scale;
    intensity = intensity+ambientLightIntensity;

	float perscieved_intensity = log(1+intensity)/log(1+lightIntensity);
	if (perscieved_intensity > 1.0) {
		perscieved_intensity = 1.0;
	} else if (perscieved_intensity < 0) {
		perscieved_intensity = 0;
	}

    vec4 outColour_hsv = rgb_to_hsv(outColour);
    outColour_hsv.z = perscieved_intensity;
    outColour = hsv_to_rgb(outColour_hsv);
}