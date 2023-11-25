$(if $(XML_Pipeline_Template),,$(error makefile requires XML_Pipeline_Template))

ExpressionsToRawSExprSrcDir := expressions_to_raw_sexpr
ExpressionsToRawSExprXMLWorkDir := $(XMLPipelineWorkDir)/$(ExpressionsToRawSExprSrcDir)

$(eval $(call XML_Pipeline_Template,$(ExpressionsToRawSExprSrcDir),$(ExpressionsToRawSExprXMLWorkDir),expressions,symbols))

$(eval $(call XML_Pipeline_Template,$(ExpressionsToRawSExprSrcDir),$(ExpressionsToRawSExprXMLWorkDir),symbols,raw_sexpr))
