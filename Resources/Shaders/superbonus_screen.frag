uniform sampler2D texture;
uniform float time;

void main()
{
	gl_FragColor = gl_Color * texture2D(texture, gl_TexCoord[0].xy) * vec4(1,1,1,0.8);
}