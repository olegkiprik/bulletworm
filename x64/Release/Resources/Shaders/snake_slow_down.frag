uniform float time;

void main()
{
	gl_FragColor = gl_Color + vec4(0, 0.3, 0.5, 0) * vec4(cos(7.0*time));
}