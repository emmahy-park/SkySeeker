#version 330

// From vertex shader
in vec2 texcoord;
in vec2 fragTexcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;

uniform float u_time;
uniform bool u_damaged;
uniform float u_health;
uniform bool u_ishealthbar;
uniform bool u_dashed;

// Output color
layout(location = 0) out vec4 color;

void main()
{
	float flashFrequency = 3.0;
	float flashIntensity;
	vec4 finalColor;

	vec4 baseColor = vec4(fcolor, 1.0) * texture(sampler0, fragTexcoord);

	if (u_ishealthbar) {
		if (fragTexcoord.x > u_health) {
			discard;
		}
		color = vec4(1.0, 0.0, 0.0, 1.0); // Red
	}

	else {
			// M1 Creative Element: Simple rendering effects (flashing effect on attacked)
			if (u_damaged) {
				float flashIntensity = 0.5 * (sin(u_time * flashFrequency * 6.28) + 1.0); // oscillates between 0 and 1
				finalColor = baseColor * (1.0 - flashIntensity) + vec4(1.0, 0.0, 0.0, (texture(sampler0, fragTexcoord)[3])) * flashIntensity; // flash color
        
        if (u_dashed) {
			    flashIntensity = 1.0;
			    finalColor = baseColor * (1.0 - flashIntensity) + vec4(1.0, 1.0, 1.0, (texture(sampler0, fragTexcoord)[3])) * flashIntensity; // white color
		    }
			}
			else {
				finalColor = baseColor;
			}
			
			color = finalColor;
	}
}
