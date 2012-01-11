// Copyright 2011 Google Inc. All Rights Reserved.

#ifndef ART_SRC_OAT_FILE_H_
#define ART_SRC_OAT_FILE_H_

#include <vector>

#include "constants.h"
#include "dex_file.h"
#include "mem_map.h"
#include "oat.h"
#include "object.h"

namespace art {

class OatFile {
 public:

  // Returns an OatFile name based on a DexFile location
  static std::string DexFilenameToOatFilename(const std::string& location);

  // Open an oat file. Returns NULL on failure.  Requested base can
  // optionally be used to request where the file should be loaded.
  static OatFile* Open(const std::string& filename,
                       const std::string& strip_location_prefix,
                       byte* requested_base);

  ~OatFile();

  const std::string& GetLocation() const {
    return location_;
  }

  const OatHeader& GetOatHeader() const;

  class OatDexFile;

  class OatMethod {
   public:
    // Link Method for execution using the contents of this OatMethod
    void LinkMethodPointers(Method* method) const;

    // Link Method for image writing using the contents of this OatMethod
    void LinkMethodOffsets(Method* method) const;

    uint32_t GetCodeOffset() const {
      return code_offset_;
    }
    size_t GetFrameSizeInBytes() const {
      return frame_size_in_bytes_;
    }
    uint32_t GetCoreSpillMask() const {
      return core_spill_mask_;
    }
    uint32_t GetFpSpillMask() const {
      return fp_spill_mask_;
    }
    uint32_t GetMappingTableOffset() const {
      return mapping_table_offset_;
    }
    uint32_t GetVmapTableOffset() const {
      return vmap_table_offset_;
    }
    uint32_t GetInvokeStubOffset() const {
      return invoke_stub_offset_;
    }

    const void* GetCode() const {
      return GetOatPointer<const void*>(code_offset_);
    }
    const uint32_t* GetMappingTable() const {
      return GetOatPointer<const uint32_t*>(mapping_table_offset_);
    }
    const uint16_t* GetVmapTable() const {
      return GetOatPointer<const uint16_t*>(vmap_table_offset_);
    }
    const Method::InvokeStub* GetInvokeStub() const {
      return GetOatPointer<const Method::InvokeStub*>(invoke_stub_offset_);
    }

    ~OatMethod();

    // Create an OatMethod with offsets relative to the given base address
    OatMethod(const byte* base,
              const uint32_t code_offset,
              const size_t frame_size_in_bytes,
              const uint32_t core_spill_mask,
              const uint32_t fp_spill_mask,
              const uint32_t mapping_table_offset,
              const uint32_t vmap_table_offset,
              const uint32_t invoke_stub_offset);

   private:
    template<class T>
    T GetOatPointer(uint32_t offset) const {
      if (offset == 0) {
        return NULL;
      }
      return reinterpret_cast<T>(base_ + offset);
    }

    const byte* base_;

    uint32_t code_offset_;
    size_t frame_size_in_bytes_;
    uint32_t core_spill_mask_;
    uint32_t fp_spill_mask_;
    uint32_t mapping_table_offset_;
    uint32_t vmap_table_offset_;
    uint32_t invoke_stub_offset_;

    friend class OatClass;
  };

  class OatClass {
   public:
    Class::Status GetStatus() const;

    // get the OatMethod entry based on its index into the class
    // defintion. direct methods come first, followed by virtual
    // methods. note that runtime created methods such as miranda
    // methods are not included.
    const OatMethod GetOatMethod(uint32_t method_index) const;
    ~OatClass();

   private:
    OatClass(const OatFile* oat_file,
             Class::Status status,
             const OatMethodOffsets* methods_pointer);

    const OatFile* oat_file_;
    const Class::Status status_;
    const OatMethodOffsets* methods_pointer_;

    friend class OatDexFile;
  };

  class OatDexFile {
   public:
    const DexFile* OpenDexFile() const;
    const OatClass* GetOatClass(uint32_t class_def_index) const;

    const std::string& GetDexFileLocation() const {
      return dex_file_location_;
    }

    uint32_t GetDexFileChecksum() const {
      return dex_file_checksum_;
    }

    ~OatDexFile();
   private:
    OatDexFile(const OatFile* oat_file,
               std::string dex_file_location,
               uint32_t dex_file_checksum,
               byte* dex_file_pointer,
               const uint32_t* oat_class_offsets_pointer);

    const OatFile* oat_file_;
    std::string dex_file_location_;
    uint32_t dex_file_checksum_;
    const byte* dex_file_pointer_;
    const uint32_t* oat_class_offsets_pointer_;

    friend class OatFile;
    DISALLOW_COPY_AND_ASSIGN(OatDexFile);
  };

  const OatDexFile* GetOatDexFile(const std::string& dex_file_location,
                                  bool warn_if_not_found = true) const;
  std::vector<const OatDexFile*> GetOatDexFiles() const;

  size_t GetSize() const {
    return GetLimit() - GetBase();
  }

 private:
  explicit OatFile(const std::string& filename);
  bool Read(const std::string& filename, byte* requested_base);

  const byte* GetBase() const;
  const byte* GetLimit() const;

  // The oat file name.
  //
  // The image will embed this to link its associated oat file.
  const std::string location_;

  // backing memory map for oat file
  UniquePtr<MemMap> mem_map_;

  typedef std::map<std::string, const OatDexFile*> Table;
  Table oat_dex_files_;

  friend class OatClass;
  friend class OatDexFile;
  friend class OatDump;  // For GetBase and GetLimit
  DISALLOW_COPY_AND_ASSIGN(OatFile);
};

}  // namespace art

#endif  // ART_SRC_OAT_WRITER_H_
