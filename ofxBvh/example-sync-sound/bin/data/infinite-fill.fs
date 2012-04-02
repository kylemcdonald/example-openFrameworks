uniform sampler2DRect src_tex_unit0, fillTexture;

const int patternCount = 38; // how many infinite fills in the texture
const int patternSize = 8; // how big the infinite fill patterns are

// lower-upper [suggestion], description
uniform float palette; // 0-1 [0], which subsection of the patterns to use
uniform float variation; // 0-1 [1], how many of the patterns to use
uniform float stretch; // 0-1 [0], how much brightness affects offset
uniform float patternCrop; // 0-1 [1], where to crop the patternSize
uniform bool useOriginal; // 0/1 [0] use original colors
uniform bool invert; // 0/1 [0] use original colors as blacks or whites
uniform bool useHue; // 0/1 [0] use lightness or use hue for pattern selection
uniform bool preserveBlackWhite; // 0/1 [1] preserve blacks and whites in original image

// utils
float max(vec3 color) {return max(max(color.r, color.g), color.b);}
float min(vec3 color) {return min(min(color.r, color.g), color.b);}
float lightness(vec3 color) {return (color.r + color.g + color.b) / 3.;}
float hue(vec3 color) {
	float max = max(color);
	float min = min(color);
	if(max == min) {
		return 0.;
	}
	float hue, range = max - min;
	if(color.r == max) {
		hue = (color.g - color.b) / range;
		if(hue < 0.) {
			hue += 6.;
		}
	} else if (color.g == max) {
		hue = 2. + (color.b - color.r) / range;
	} else {
		hue = 4. + (color.r - color.g) / range;
	}
	return hue / 6.;
}

void main() {
	vec4 label = texture2DRect(src_tex_unit0, gl_TexCoord[0].xy);
	float lightness = lightness(label.rgb);
	if(preserveBlackWhite && lightness == 0.) {
		gl_FragColor = label;
	} else if(preserveBlackWhite && lightness == 1.) {
		gl_FragColor = label;
	} else {
		float base;
		if(useHue) {
			base = hue(label.rgb);
		} else {
			base = lightness;
		}
		float index = mod(base * variation + palette, 1.);
		int choice = int(index * float(patternCount));
		vec2 shift = vec2(stretch * lightness);
		vec2 intraPosition = mod(shift + gl_FragCoord.xy, vec2(int(float(patternSize) * patternCrop)));
		vec2 interPosition = vec2(0, patternSize * choice);
		gl_FragColor = gl_Color * texture2DRect(fillTexture, interPosition + intraPosition);
		if(useOriginal) {
			if((invert && gl_FragColor.r > 0.) ||
				(!invert && gl_FragColor.r < 1.)) {
				gl_FragColor = label;
			}
		}
	}
}
