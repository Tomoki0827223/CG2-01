struct PixelShaderOutput
{

    float32_t4 coler : SV_TARGET0;
	
};



PixelShaderOutput main()
{
    PixelShaderOutput output;
	
	output.coler = float32_t4 (1.0f, 1.0f, 1.0f, 1.0f);
	
    return output;
}