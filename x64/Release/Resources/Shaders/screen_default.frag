uniform sampler2D texture;
uniform float time;

void main()
{
	float saturation = 0.9;
	vec3 satVec = vec3(saturation);
	float value = (4.0 + sin(time)) / 5.0;
	vec4 color = gl_Color * texture2D(texture, gl_TexCoord[0].xy);
	gl_FragColor = vec4(vec3(0.2) + satVec*(color.rgb - vec3(0.2)) * value, color.a);
}