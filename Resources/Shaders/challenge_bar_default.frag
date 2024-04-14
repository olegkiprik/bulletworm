uniform float time;

void main()
{
	float value = (4.0+sin(time))/5.0;
	vec4 color = gl_Color;
	gl_FragColor = vec4(color.rgb * value, color.a);
}