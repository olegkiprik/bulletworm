uniform sampler2D texture;
uniform float time;

void main()
{
	float saturation = 0.7;
	float left = 0.4;

	vec3 satVec = vec3(saturation);
	vec3 leftVec = vec3(left);
	
	vec4 color = gl_Color;
	vec3 lerp = leftVec + satVec * (color.rgb - leftVec);
	
	gl_FragColor = vec4(lerp, color.a);
}