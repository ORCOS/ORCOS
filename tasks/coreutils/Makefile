TARGET=armv7
UTILS=cp crc32 

all:	
	@mkdir -p $(TARGET)
	@for dir in $(UTILS); do \
	  make -s -C $$dir/ TARGET=$(TARGET) POST_BUILD="cp $(TARGET)/$$dir ../$(TARGET)/" all;\
	done;
	
clean:
	@for dir in $(UTILS); do \
	  echo Cleaning $$dir;\
	  make -s -C $$dir/ clean;\
	done;
	@rm -fr $(TARGET)
