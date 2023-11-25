
# Expects the input file to be called $(XMLPipelineWorkDir)/raw_sexpr_to_expressions/whatever.raw_sexpr.xml
# and the output file to be called $(XMLPipelineWorkDir)/raw_sexpr_to_expressions/whatever.expressions.xml

$(if $(XML_Pipeline_Template),,$(error makefile requires XML_Pipeline_Template))

RawSExprSrcDir := raw_sexpr_to_expressions
RawSExprToXMLWorkDir := $(XMLPipelineWorkDir)/$(RawSExprSrcDir)

$(eval $(call XML_Pipeline_Template,$(RawSExprSrcDir),$(RawSExprToXMLWorkDir),raw_sexpr,list))
$(eval $(call XML_Pipeline_Template,$(RawSExprSrcDir),$(RawSExprToXMLWorkDir),list,symbols))
$(eval $(call XML_Pipeline_Template,$(RawSExprSrcDir),$(RawSExprToXMLWorkDir),symbols,expressions))
