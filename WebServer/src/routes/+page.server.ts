import { fetchControllerStatus } from '$lib/server/controller';
import type { PageServerLoad } from './$types';

export const load: PageServerLoad = async ({ fetch }) => {
	return {
		initialStatus: await fetchControllerStatus(fetch)
	};
};
