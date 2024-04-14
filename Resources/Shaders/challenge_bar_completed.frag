uniform float time;

void main()
{
	gl_FragColor = gl_Color + vec4(0.2, 0.2, 0.2, 0)*vec4(cos(10.0*time));
}