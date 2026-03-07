uniform float time;

void main()
{
	vec4 colored = gl_Color;
	float value = (colored.r+colored.g+colored.b)/3.0;
	gl_FragColor = vec4(value, value, value, colored.a);
}