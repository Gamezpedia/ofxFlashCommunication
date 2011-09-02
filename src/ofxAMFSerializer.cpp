#include "ofxAMFSerializer.h"
#include "ofxAMFMessage.h"
#include "ofMain.h" // only for debugging

ofxAMFSerializer::ofxAMFSerializer() {
}

ofxAMFSerializer::~ofxAMFSerializer() {
	
}

// serializes a packet
IOBuffer ofxAMFSerializer::serialize(ofxAMFPacket& packet) {
	IOBuffer buffer;
	buffer.setup();
	
	// write version
	buffer.storeBigEndianUInt16(packet.getClientVersion());
	
	// headers
	uint16_t header_count = packet.getNumHeaders();
	buffer.storeBigEndianUInt16(header_count);
	for(uint16_t i = 0; i < header_count; ++i) {
		printf("@todo implement serializing of headers.\n");
	}
	
	// messages
	uint16_t message_count = packet.getNumMessages();
	buffer.storeBigEndianUInt16(message_count);
	for(uint16_t i = 0; i < message_count; ++i) {
		// write target and response uri.
		ofxAMFMessage* message = packet.getMessage(i);
		writeUTF(buffer, message->getTargetURI());
		writeUTF(buffer, message->getResponseURI());
		
		// write the data
		Dictionary data = message->getData();
		string js;
		data.serializeToJSON(js);
		cout << "----------- packetize ----------" << endl;
		cout << js << endl;
		cout << "--------------------------------" << endl;
		cout << endl;


		if(packet.isAMF3()) {
			printf("store afm3\n");
			IOBuffer tmp_buffer;
			tmp_buffer.storeByte(AMF0_AMF3_OBJECT);

			// we write the contents to a tmp buffer, so we can get the size
			// of it which we are supposed to add before the actually data.
			
			writeAMF3Type(tmp_buffer, data);
			
			buffer.storeBuffer(tmp_buffer);
		}
		else {
			printf("store amf0\n");
			writeType(buffer, data);
		}
		
	}
	
	return buffer;
}
/*
  $this->writeInt(0); //  write the version (always 0)
        $count = count($this->packet->headers);
        l('header count:' .$count);
       
		$count = count($this->packet->messages);
        $this->writeInt($count); // write the Message  count
        for ($i = 0; $i < $count; $i++) {
            $this->resetReferences();
            //write body.
            $message = $this->packet->messages[$i];
            $this->currentMessage = & $message;
            $this->writeUTF($message->targetUri);
            $this->writeUTF($message->responseUri);
            //save the current buffer, and flush it to write the Message
            $tempBuf = $this->outBuffer;
            $this->outBuffer = "";
            $this->writeData($message->data);
            $serializedMessage = $this->outBuffer;
            $this->outBuffer = $tempBuf;
            $this->writeLong(strlen($serializedMessage));
            $this->outBuffer .= $serializedMessage;
        }

*/

void ofxAMFSerializer::writeAMF3Object(IOBuffer& buffer, Dictionary& source) {
	
}

void ofxAMFSerializer::writeAMF3Type(IOBuffer& buffer, Dictionary& input) {
	switch(input.type) {
		case D_NULL: {

			break;
		}
		
		case D_MAP: {
			if(input.isArray()) {
				writeAMF3Object(buffer, input);
			}
			else {
				printf("@todo write amf3: write a object map\n");
			}
			break;
		}
		
		case D_STRING: {

			break;
		}
		
		default:{
			printf("write amf3: Unhandled input type.\n");
			break;
		};
	}
	
	/*
	 D_NULL		=	0
	,D_BOOL		= 	1
	
	,D_INT8		=	2
	,D_INT16	=	3
	,D_INT32	=	4
	,D_INT64	=	5
	
	,D_UINT8	=	6
	,D_UINT16	=	7
	,D_UINT32	=	8
	,D_UINT64	=	9
	
	,D_DOUBLE	= 	10
	,D_STRING	=	11
	,D_MAP		= 	12
	*/

}

void ofxAMFSerializer::writeAMF3Array(IOBuffer& buffer, Dictionary& source) {
	buffer.storeByte(AMF3_ARRAY); 
	
	Dictionary s = source;
	uint32_t dense_size = s.getMapDenseSize();
	printf("dense size of array: %u\n", dense_size);
	
	// @todo figure out why this is done in the crtmp server.
	for(uint32_t i = 0; i < dense_size; ++i) {
		s.removeAt(i);	
	}
	
	writeU29(buffer, (dense_size << 1) | 0x01);
}

void ofxAMFSerializer::writeUTF(IOBuffer& buffer, string value) {
	uint16_t length = (uint16_t)value.length();
	buffer.storeBigEndianUInt16(length);
	buffer.storeString(value);
}

void ofxAMFSerializer::writeType(IOBuffer& buffer, Dictionary& input) {
	switch(input.type) {
		case D_NULL: {
			writeNull(buffer, input);
			break;
		}
		
		case D_MAP: {
			if(input.isArray()) {
				writeArray(buffer, input);
			}
			else {
				printf("@todo write a object map\n");
			}
			break;
		}
		
		case D_STRING: {
			writeString(buffer, input);
			break;
		}
		
		default:{
			printf("Unhandled input type.\n");
			break;
		};
	}
	
	/*
	 D_NULL		=	0
	,D_BOOL		= 	1
	
	,D_INT8		=	2
	,D_INT16	=	3
	,D_INT32	=	4
	,D_INT64	=	5
	
	,D_UINT8	=	6
	,D_UINT16	=	7
	,D_UINT32	=	8
	,D_UINT64	=	9
	
	,D_DOUBLE	= 	10
	,D_STRING	=	11
	,D_MAP		= 	12
	*/
}

void ofxAMFSerializer::writeNull(IOBuffer& buffer, Dictionary& source) {
}

void ofxAMFSerializer::writeArray(IOBuffer& buffer, Dictionary& source) {
	
}

bool ofxAMFSerializer::writeU29(IOBuffer& buffer, uint32_t value) {
	uint32_t temp = htonlex(value); // convert to big endian (network byte order)
	uint8_t* temp_ptr = (uint8_t*)&temp;
	
	if((0x00000000 <= value) && (value <= 0x0000007f)) {
		printf("Given value: %u\n", value);
		printf("store... : %02X\n", temp_ptr[3]);
		buffer.storeUInt8(temp_ptr[3]);
		return true;
	}
	return false;
/*
uint32_t temp = EHTONL(value);
	uint8_t *pBuffer = (uint8_t*) & temp;
	if ((0x00000000 <= value) && (value <= 0x0000007f)) {
		buffer.ReadFromRepeat(pBuffer[3], 1);
		return true;
	} else if ((0x00000080 <= value) && (value <= 0x00003fff)) {
		buffer.ReadFromRepeat(((pBuffer[2] << 1) | (pBuffer[3] >> 7)) | 0x80, 1);
		buffer.ReadFromRepeat(pBuffer[3]&0x7f, 1);
		return true;
	} else if ((0x00004000 <= value) && (value <= 0x001fffff)) {
		buffer.ReadFromRepeat(((pBuffer[1] << 2) | (pBuffer[2] >> 6)) | 0x80, 1);
		buffer.ReadFromRepeat(((pBuffer[2] << 1) | (pBuffer[3] >> 7)) | 0x80, 1);
		buffer.ReadFromRepeat(pBuffer[3]&0x7f, 1);
		return true;
	} else if ((0x0020000 <= value) && (value <= 0x01fffffff)) {
		buffer.ReadFromRepeat(((pBuffer[0] << 2) | (pBuffer[1] >> 6)) | 0x80, 1);
		buffer.ReadFromRepeat(((pBuffer[1] << 1) | (pBuffer[2] >> 7)) | 0x80, 1);
		buffer.ReadFromRepeat(pBuffer[2] | 0x80, 1);
		buffer.ReadFromRepeat(pBuffer[3], 1);
		return true;
	}
*/
}

void ofxAMFSerializer::writeString(IOBuffer& buffer, Dictionary& source) {
}

void ofxAMFSerializer::writeUint8(IOBuffer& buffer, uint8_t value) {
	
}

// @todo pass Dictionary as input
ofxAMFPacket ofxAMFSerializer::deserialize(IOBuffer& buffer) {
	ofxAMFPacket packet;
	buffer.printHex();

	// get version.
	uint16_t version = buffer.consumeLittleEndianUInt16();
	packet.setClientVersion(version);
	
	// get headers count.
	uint16_t header_count = buffer.consumeLittleEndianUInt16();
	
	// parsing headers
	for(uint16_t i = 0; i < header_count; ++i) {
		printf("@todo implement header parsing\n");
	}
	
	uint16_t message_count = buffer.consumeLittleEndianUInt16();
	
	cout << "version: " << version << endl;
	cout << "header count: " << header_count << endl;
	cout << "message count: "<< message_count << endl;
	
	string target_uri = "";
	string response_uri = "";
	for(uint16_t i = 0; i < message_count; ++i) {
		if(!readUTF(buffer, target_uri) || !readUTF(buffer, response_uri)) {
			printf("error while parsing target and/or response uri");
			continue;
		}
		cout << "target uri: " << target_uri << endl;
		cout << "response uri: " << response_uri << endl;

		ofxAMFMessage* message = new ofxAMFMessage();
		message->setTargetURI(target_uri);
		message->setResponseURI(response_uri);
		packet.addMessage(message);	
		
		// body length (can be ignored): buffer.ignore(4)
		//buffer.ignore(4);
		uint32_t message_length = buffer.consumeLittleEndianUInt32();
		message->setMessageLength(message_length);
		
		uint8_t value_type = buffer.consumeUInt8();
		printf("MESSAGE VALUE TYPE: %02X\n", value_type);
		Dictionary result = readType(buffer, value_type);
		message->setData(result);
	
		// debug
		string json;
		if(result.serializeToJSON(json)) {
			cout << "---------------------json-------------\n";
			cout << json << endl;
			cout << "+++++++++++++++++++++++++++++++++++++++\n\n";
		}
		
		// tmp: here we create the response (temp);
		// the response should be formatted the same as the request, so 
		// again the request and response uris version, headers count,
		// message count etc..
		//IOBuffer output;
		//output.setup();
		//serialize(output, result);
	}
	return packet;
}


Dictionary ofxAMFSerializer::readType(IOBuffer& buffer, int type) {
	printf("readType: %02X\n", type);

	// @todo order by most frequest type
	Dictionary result;
	switch(type) {
		// AMF 0
		//------------------------------------------
		case AMF0_AMF3_OBJECT: {
			return readAMF3Type(buffer);
			break;
		}
		
		case AMF0_NUMBER: {
			break;
		}
		case AMF0_BOOLEAN: {
			break;
		}
		case AMF0_SHORT_STRING: {
			result = readString(buffer);	
			break;
		}
		case AMF0_OBJECT: {
			break;
		}
		case AMF0_NULL: {
			break;
		}
		case AMF0_UNDEFINED: {
			break;
		}
		case AMF0_MIXED_ARRAY: {
			break;
		}
		case AMF0_ARRAY: {
			result = readArray(buffer);
			break;
		}
		case AMF0_TIMESTAMP: {
			break;
		}
		case AMF0_LONG_STRING: {
			break;
		}
		case AMF0_TYPED_OBJECT: {
			break;
		}
		default: {
			printf("amf serializer, unknown type.\n");
			result = false;
			break;
		}
	};
	return result;
}

Dictionary ofxAMFSerializer::readAMF3Type(IOBuffer& buffer) {
	uint8_t type = buffer.consumeUInt8();
	printf("readType: %02X\n", type);

	// @todo order by most frequest type
	Dictionary result;
	switch(type) {
		// AMF 3 
		//------------------------------------------
		case AMF3_UNDEFINED: {
			break;
		}  
		case AMF3_NULL: {
			break;
		} 
		case AMF3_FALSE: {
			break;
		} 
		case AMF3_TRUE: {
			break;
		} 
		case AMF3_INTEGER: {
			break;
		} 
		case AMF3_DOUBLE: {
			break;
		} 
		case AMF3_STRING: {
			result = readAMF3String(buffer);
			break;
		} 
		case AMF3_XMLDOC: {
			break;
		} 
		case AMF3_DATE: {
			break;
		} 
		case AMF3_ARRAY: {
			result = readAMF3Array(buffer);
			break;
		} 
		case AMF3_OBJECT: {
			break;
		} 
		case AMF3_XML: {
			break;
		} 
		case AMF3_BYTEARRAY: {
			break;
		} 
		default: {
			printf("amf serializer, unknown type.\n");
			result = false;
			break;
		}
	};
	return result;

}

Dictionary ofxAMFSerializer::readObject(IOBuffer& buffer) {

}

Dictionary ofxAMFSerializer::readString(IOBuffer& buffer) {
	Dictionary result;
	string str;
	if(readUTF(buffer, str)) {
		result = str;
	}
	else {
		result = false;
	}
	return result;
}

// numeric keyed Actionscript Array
Dictionary ofxAMFSerializer::readArray(IOBuffer& buffer) {
	Dictionary result;
	uint32_t num_els = buffer.consumeLittleEndianUInt32();
	cout << "Number of elements in array: " << num_els << endl;
	for(uint32_t i = 0; i < num_els; ++i) {
		uint8_t type = buffer.consumeUInt8();
		Dictionary array_el = readType(buffer, type);
		result.pushToArray(array_el);
	}
	return result;
}


Dictionary ofxAMFSerializer::readAMF3Array(IOBuffer& buffer) {
	Dictionary result;
	
	// check if we have a reference handle
	uint32_t ref;
	if(!readU29(buffer, ref)) {
		result = false;
		printf("error reading u29\n");
		return result;
	}

	// did we get an array reference	
	if( (ref & 0x01) == 0) {
		printf("We have a reference to an array: %u", ref >> 1); 
		printf("@todo handle array reference.\n");
		return result;
	}
	
	// read array
	while(true) {
		Dictionary key = readAMF3String(buffer);
		cout << "Got amf3 string: " << (string)key << endl;
		if((string)key == "") {
			break;
		}
		else {
			printf("@todo unhandled portion in read array ..!\n");
			break; // @todo implement
		}
	}
	
	// read dense portion
	uint32_t dense_size = ref >> 1;
	for(uint32_t i = 0; i < dense_size; ++i) {
		result[(uint32_t)i] = readAMF3Type(buffer);
	}
	result.isArray(true);
	return result;
}

Dictionary ofxAMFSerializer::readAMF3String(IOBuffer& buffer) {
	Dictionary result;
	uint32_t ref;
	if(!readU29(buffer, ref)) {
		result = false;
		cout << "erro reading amf3 string." << endl;
		return result;
	}
	
	// did we get an reference.
	if( (ref & 0x01) == 0) {
		printf("@todo handle string reference\n");
		result = false;
		return result;
	}
	else {
		uint32_t len = ref >> 1;
		if(len == 0) {
			result = "";
		}	
		else {
			result = buffer.consumeString(len);
		}
	}
	return result;
}

// Read next data as UTF string. (utf precedes with a 16bit length.)
bool ofxAMFSerializer::readUTF(IOBuffer& buffer, string& result) {
	uint16_t utf_length = buffer.consumeLittleEndianUInt16();
	cout << "UTF length: " << utf_length << endl;	
	if(utf_length > 0) {
		result = buffer.consumeString(utf_length);
		return true;
	}
	return false;
}


bool ofxAMFSerializer::readU29(IOBuffer& buffer, uint32_t& value) {
	value = 0;
	for(uint32_t i = 0; i < 4; ++i) {
		uint8_t byte = buffer.consumeUInt8();
		if(i != 3) {
			value = (value << 7) | (byte & 0x7f);
		}
		else {
			value = (value << 8) | byte;
		}
		if((byte & 0x80) == 0) {
			break;
		}
	}
	cout << "readU29: " << value << endl;
	return true;
}