uniform sampler2D texture;
uniform float time;

void main()
{
	gl_FragColor = gl_Color + vec4(0,0,0.1,0) * vec4(4.0+2.0*cos(10.0*time));
}