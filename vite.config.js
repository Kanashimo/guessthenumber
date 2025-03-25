import { defineConfig} from 'vite'
export default defineConfig({
    root: 'src/client',
    publicDir: 'static',
    server: {
        port: 80,
    },
});