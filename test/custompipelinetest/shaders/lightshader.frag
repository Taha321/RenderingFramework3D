#version 450


layout(location = 0) in vec4 inWorldPosition;
layout(location = 1) in vec4 inObjectPosition;


layout(location = 0) out vec4 outColour;

layout(set = 0, binding = 1) uniform LightInfo {
	vec4 LightColour;
	float LightIntensity;
	float LightRadius;
};

//enable uniformShaderInputLayout.ObjectInputs.useCamTransform in pipeline configuration
layout(set = 0, binding = 2) uniform CameraUniformBufferObject {
	mat4 camTransform;
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

float calc_acc_intensity(vec3 view_dir, vec3 to_light) {
	float light_dot_view = dot(to_light, view_dir);
	float intersection_width = length(2.0f*light_dot_view*view_dir);

    float r_sqr = dot(to_light, to_light);
    float light_dot_view_sqr = light_dot_view*light_dot_view;
	float den = sqrt(r_sqr-light_dot_view_sqr);
	if(den > 0.001) {
        float result = 0;
		float a = LightRadius*LightIntensity/den;
        float b = intersection_width-light_dot_view;
		result += a*(atan(b/den) - atan((-light_dot_view)/(LightRadius*den)));

        return result;
	}

	return 1.0;
}

float calc_acc_intensity_raymarch(vec3 view_dir, vec3 to_light) {
    float steps = 100;
    float total_intensity = 0;
    
	float light_dot_view = dot(to_light, view_dir);
	float intersection_width = length(2.0f*light_dot_view*view_dir);
    float to_ligth_sqr = dot(to_light, to_light);
    float step_size = intersection_width/steps;

    if(abs(light_dot_view) < 0.001) {
        return 1;
    }
    if(LightRadius > 0.001) {
        for(int i = 0; i < steps; i++) {
            float t = i*step_size;

            float x = (t-light_dot_view)/LightRadius;
            float den = (x*x - light_dot_view*light_dot_view + to_ligth_sqr);
            total_intensity += max(step_size*dot(normalize(to_light-t*view_dir),view_dir)*LightIntensity / den, 0);
        }
    }

    return total_intensity;
}


void main() {
	vec3 view_dir = normalize(inWorldPosition.xyz - camTransform[3].xyz);
	vec3 to_light = -inObjectPosition.xyz;
	
	//float intensity = calc_acc_intensity(view_dir, to_light);
	float intensity = calc_acc_intensity_raymarch(view_dir, to_light);
	float perscieved_intensity = log(1+intensity)/log(1+LightIntensity);
	if (perscieved_intensity > 1.0) {
		perscieved_intensity = 1.0;
	}
	vec4 hsv = rgb_to_hsv(LightColour);
	hsv[2] = perscieved_intensity;
	hsv[3] = perscieved_intensity;

	outColour = hsv_to_rgb(hsv);
}