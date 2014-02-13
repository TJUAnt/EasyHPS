#/*=====================================================================*/
#/*                       TJUHPCLAB Project                             */
#/*                (http://cs.tju.edu.cn/orgs/hpclab)                   */
#/*                                                                     */
#/*             EasyHPS - Copyright © 2009-2011 TJUHPCLAB               */
#/*                                                                     */
#/* Contact: Tang shanjiang, School of Computer Science & Technology,   */
#/*                                               Tianjin University.   */
#/*          Chao Sun,       School of Computer Science & Technology,   */
#/*                                               Tianjin University.   */
#/*          Jun Du,		 School of Computer Science & Technology,   */
#/*                                               Tianjin University.   */
#/* Emails:  tashj@tju.edu.cn; schsch321@tju.edu.cn;                    */
#/*          tjudujun@tju.edu.cn										*/
#/*                                                                     */
#/* GPL version 3.0 applies                                             */
#/*=====================================================================*/


include ./Config.mk
include ./EasyHPS.mk

# user object files
OBJS := $(patsubst %.cpp,%.o,$(SRCS))

# EasyHPS object files
HPS_OBJS := $(patsubst %.cpp,%.o, $(HPS_SRCS))

# Add bin path
OBJS := $(addprefix $(BIN),$(OBJS))
HPS_OBJS := $(addprefix $(BIN),$(HPS_OBJS))
APP := $(addprefix $(BIN), $(APP))

# All Target
all : $(APP)

# Tool invocations
$(APP) : $(OBJS) $(HPS_OBJS)
	@echo 'Building target: ' $@
	-@mkdir -p $(dir $@)
	$(CC) $(LIBS) -o $(APP) $(OBJS) $(HPS_OBJS)
	@echo 'Finished building target: ' $@
	@echo ' '

# Each subdirectory must supply rules for building sources it contributes
$(BIN)%.o: %.cpp
	@echo 'Building file: ' $<
	-@mkdir -p $(dir $@)
	$(CC) -O3 -Wall -c -o "$@" "$<"
	@echo 'Finished building: ' $<
	@echo ' '

# Other targets
clean:
	-rm -f $(OBJS) $(HPS_OBJS) $(APP)
	-rm run_log
	-@echo ' '

.PHONY: all clean

