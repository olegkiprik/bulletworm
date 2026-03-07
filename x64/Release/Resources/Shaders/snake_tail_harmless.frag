uniform sampler2D texture;
uniform float time;

void main()
{
	gl_FragColor = gl_Color.brba * vec4(1, 1, 1, 0.7);
}