uniform sampler2D texture;
uniform float time;

void main()
{
	vec4 color = gl_Color * texture2D(texture, gl_TexCoord[0].xy);
	gl_FragColor = vec4(color.r/2.0,color.g/2.0,color.b/2.0,color.a);
}