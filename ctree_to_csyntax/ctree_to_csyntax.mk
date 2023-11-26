$(if $(XML_Pipeline_Template),,$(error makefile requires XML_Pipeline_Template))

ctree_to_csyntax_srcdir := ctree_to_csyntax
ctree_to_csyntax_workdir := $(XMLPipelineWorkDir)/$(ctree_to_csyntax_srcdir)

$(eval $(call XML_Pipeline_Template,$(ctree_to_csyntax_srcdir),$(ctree_to_csyntax_workdir),ctree,csyntax))
